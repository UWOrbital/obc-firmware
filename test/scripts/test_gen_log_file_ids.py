"""Exercise file discovery and CMake's automatic per-source ID updates."""

import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
GENERATOR = REPO_ROOT / "scripts" / "gen_log_file_ids.py"
spec = importlib.util.spec_from_file_location("gen_log_file_ids", GENERATOR)
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class TestLogFileIdGenerator(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def write(self, path, content):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content)

    def run_generator(self, *args):
        return subprocess.run(
            [sys.executable, str(GENERATOR), "--repo-root", str(self.root), *args],
            check=False,
            capture_output=True,
            text=True,
        )

    def test_discovers_assertions_and_nested_wrappers_without_comment_false_positives(
        self,
    ):
        self.write(
            "obc/wrappers.h",
            """
            #define INNER(x) RETURN_IF_ERROR_CODE(x)
            #define OUTER(x) \\
                INNER(x)
            #define ALIAS OUTER
        """,
        )
        self.write("obc/direct.c", 'void f(void) { LOG_INFO("hello"); }')
        self.write("obc/wrapped.c", "void f(void) { OUTER(call()); }")
        self.write("obc/aliased.c", "void f(void) { ALIAS(call()); }")
        self.write(
            "obc/shared/hal/freertos/kernel.c", "void f(void) { configASSERT(1); }"
        )
        self.write("obc/hal.c", "void f(void) { ASSERT(1); }")
        self.write("libs/unrelated.c", "void f(void) { configASSERT(1); }")
        self.write(
            "obc/quiet.c", '// LOG_ERROR("example")\nconst char *s = "configASSERT(0)";'
        )
        found = generator.find_logging_files(self.root)
        self.assertEqual(
            found,
            sorted(
                [
                    "obc/aliased.c",
                    "obc/direct.c",
                    "obc/hal.c",
                    "obc/shared/hal/freertos/kernel.c",
                    "obc/wrapped.c",
                    "obc/wrappers.h",
                ]
            ),
        )

    def test_build_outputs_are_deterministic_and_do_not_modify_source_snapshot(self):
        self.write("obc/b.c", "LOG_ERROR_CODE(1);")
        self.write("obc/a.c", 'LOG_INFO("hello");')
        self.write("obc/shared/logging/log_file_ids.json", "source snapshot\n")
        output = self.root / "build" / "generated"
        args = ("--output-dir", str(output))
        first = self.run_generator(*args)
        self.assertEqual(first.returncode, 0, first.stderr)
        before = {
            p.name: (p.read_bytes(), p.stat().st_mtime_ns) for p in output.iterdir()
        }
        second = self.run_generator(*args)
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertEqual(
            before,
            {p.name: (p.read_bytes(), p.stat().st_mtime_ns) for p in output.iterdir()},
        )
        self.assertEqual(
            json.loads((output / "log_file_ids.json").read_text())["files"],
            ["obc/a.c", "obc/b.c"],
        )
        self.assertIn(
            '"obc/a.c"\n    "obc/b.c"', (output / "log_file_ids.cmake").read_text()
        )
        self.assertEqual(
            (self.root / "obc/shared/logging/log_file_ids.json").read_text(),
            "source snapshot\n",
        )
        self.assertEqual(self.run_generator(*args, "--check").returncode, 0)
        self.write("obc/c.c", "configASSERT(1);")
        self.assertEqual(self.run_generator(*args, "--check").returncode, 1)

    def test_legacy_default_output_and_check_still_work(self):
        self.write("obc/a.c", 'LOG_INFO("hello");')
        result = self.run_generator()
        self.assertEqual(result.returncode, 0, result.stderr)
        output = self.root / "obc/shared/logging"
        self.assertTrue((output / "obc_log_file_ids.h").is_file())
        self.assertTrue((output / "obc_log_file_ids.c").is_file())
        self.assertTrue((output / "log_file_ids.json").is_file())
        self.assertFalse((output / "log_file_ids.cmake").exists())
        self.assertEqual(self.run_generator("--check").returncode, 0)

    @unittest.skipUnless(
        shutil.which("cmake"), "CMake is required for integration test"
    )
    def test_cmake_rebuild_updates_calls_files_and_child_target_scope(self):
        # A small native project exercises our real module without downloading
        # firmware toolchains or depending on the firmware's other tests.
        self.write("scripts/gen_log_file_ids.py", GENERATOR.read_text())
        self.write(
            "cmake/log_file_ids.cmake",
            (REPO_ROOT / "cmake/log_file_ids.cmake").read_text(),
        )
        self.write(
            "obc/main.c",
            """
            #include <stdio.h>
            #define LOG_INFO(x) ((void)0)
            int child_id(void);
            int main(void) {
                LOG_INFO("hello");
                printf("%d %d %d\\n", OBC_LOG_FILE_ID, child_id(), LOG_USE_FILE_IDS);
                return 0;
            }
        """,
        )
        self.write(
            "obc/child/child.c",
            """
            #define configASSERT(x) ((void)0)
            int child_id(void) { configASSERT(1); return OBC_LOG_FILE_ID; }
        """,
        )
        self.write("obc/quiet.c", "int quiet(void) { return 0; }\n")
        self.write("obc/child/CMakeLists.txt", "add_library(child STATIC child.c)\n")
        self.write(
            "CMakeLists.txt",
            """
            cmake_minimum_required(VERSION 3.18)
            project(LogIds C)
            include(cmake/log_file_ids.cmake)
            add_subdirectory(obc/child)
            add_executable(probe obc/main.c obc/quiet.c)
            target_link_libraries(probe PRIVATE child)
            add_dependencies(probe generate-log-file-ids)
            apply_log_file_ids("${CMAKE_SOURCE_DIR}")
        """,
        )
        build = self.root / "build"
        cmake_generator = ["-G", "Ninja"] if shutil.which("ninja") else []

        def run(*args):
            result = subprocess.run(args, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            return result.stdout

        def rebuild_and_check():
            # Older Make versions compare whole seconds. Let rewritten source
            # and generated compiler flags differ from the preceding objects.
            if not cmake_generator:
                time.sleep(1.05)
            run("cmake", "--build", str(build))
            paths = json.loads(
                (build / "generated/logging/log_file_ids.json").read_text()
            )["files"]
            values = [int(value) for value in run(str(build / "probe")).split()]
            self.assertEqual(
                values,
                [
                    paths.index("obc/main.c"),
                    paths.index("obc/child/child.c"),
                    1,
                ],
            )
            return paths

        run(
            "cmake",
            "-S",
            str(self.root),
            "-B",
            str(build),
            "-DLOG_USE_FILE_IDS=ON",
            *cmake_generator,
        )
        self.assertNotIn("obc/quiet.c", rebuild_and_check())
        self.write(
            "obc/quiet.c",
            "#define LOG_INFO(x) ((void)0)\nint quiet(void) { LOG_INFO(1); return OBC_LOG_FILE_ID; }\n",
        )
        self.assertIn("obc/quiet.c", rebuild_and_check())
        self.write("obc/added.c", "configASSERT(1);\n")
        self.assertIn("obc/added.c", rebuild_and_check())
        (self.root / "obc/added.c").unlink()
        self.assertNotIn("obc/added.c", rebuild_and_check())
        self.write("obc/quiet.c", "int quiet(void) { return 0; }\n")
        self.assertNotIn("obc/quiet.c", rebuild_and_check())


if __name__ == "__main__":
    unittest.main()
