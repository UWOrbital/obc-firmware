from re import match
from typing import Final

from jinja2 import Environment, FileSystemLoader
from pydantic import BaseModel, PositiveInt, ValidationError, field_validator
from toml import load


class Task(BaseModel):
    """
    @brief Class for storing task data

    :param task_name: name of task; snake_case
    :type task_name: str
    :param stack_size: Stack size as 32-bit unsigned int
    :type stack_size: PositiveInt
    :param priority: task priority as 32-bit unsigned int; #U or macro
    :type priority: str
    :param function_stem: base name for task functions; PascalCase
    :type function_stem: str
    :param config_id_stem: config id; SNAKE_CASE
    :type config_id_stem: str
    :param conditional_enable: conditional macro; SNAKE_CASE
    :type conditional_enable: str, None
    :param task_init: whether task has init function
    :type task_init: bool
    """

    task_name: str
    stack_size: PositiveInt
    priority: str
    function_stem: str
    config_id_stem: str
    conditional_enable: str | None = None
    task_init: bool = True

    @field_validator("task_name", mode="after")
    @classmethod
    def is_snake_case(cls, name: str) -> str:
        """
        Checks if the string name is a snake_case string
        """
        if match(r"^[a-z]+(_[a-z]+)*$", name):
            return name
        raise ValueError(f"{name} must be lower snake_case")

    @field_validator("priority", mode="after")
    @classmethod
    def is_valid_priority(cls, name: str) -> str:
        """
        Checks if the string name is a valid priority (#U or macro)
        """
        if match(r"^([0-9]+U)|([A-Z]+(_[A-Z]+)*)$", name):
            return name
        raise ValueError(f"{name} must be #U or upper SNAKE_CASE")

    @field_validator("function_stem", mode="after")
    @classmethod
    def is_pascal_case(cls, name: str) -> str:
        """
        Checks if the string name is a PascalCase string
        """
        if match(r"^([A-Z][a-z]*)+$", name):
            return name
        raise ValueError(f"{name} must be PascalCase")

    @field_validator("config_id_stem", "conditional_enable", mode="after")
    @classmethod
    def is_upper_snake_case(cls, name: str) -> str:
        """
        Checks if the string name is a SNAKE_CASE string
        """
        if match(r"^[A-Z]+(_[A-Z]+)*$", name):
            return name
        raise ValueError(f"{name} must be upper SNAKE_CASE")


C_TEMPLATE_FILENAME: Final[str] = "config_skeleton.c.jinja"
H_TEMPLATE_FILENAME: Final[str] = "header_skeleton.h.jinja"
CONFIG_FILENAME: Final[str] = "scheduler_config.toml"
C_FILENAME: Final[str] = "obc_scheduler_config.c"
H_FILENAME: Final[str] = "obc_scheduler_config.h"


def main() -> None:
    """Entry point to script"""
    env = Environment(
        loader=FileSystemLoader(""),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )

    c_template = env.get_template(C_TEMPLATE_FILENAME)
    h_template = env.get_template(H_TEMPLATE_FILENAME)
    tasks: list[Task] = []

    context = {
        "tasks": tasks,
    }

    with open(CONFIG_FILENAME) as config:
        toml_data = load(config)

    is_error: bool = False

    for t in toml_data["tasks"]:
        try:
            tasks.append(Task(**t))
        except ValidationError as e:
            print(e)
            is_error = True

    if is_error:
        exit(1)

    with open(C_FILENAME, "w") as c_output:
        c_output.write(c_template.render(context))
        print(f"Generated new scheduler config at {C_FILENAME}")

    with open(H_FILENAME, "w") as h_output:
        h_output.write(h_template.render(context))
        print(f"Generated new header file at {H_FILENAME}")


if __name__ == "__main__":
    main()
