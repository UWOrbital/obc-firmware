import io
import sys
import threading
from collections.abc import Callable
from sys import argv
from typing import cast

from serial import Serial, SerialException
from textual import on
from textual.app import App, ComposeResult
from textual.containers import HorizontalGroup, HorizontalScroll, ScrollableContainer, VerticalScroll
from textual.reactive import reactive
from textual.widget import Widget
from textual.widgets import Button, DataTable, Input, Label, Static

from gs.backend.gs_cli.ground_station_cli import GroundStationShell

if len(argv) == 2:
    COM_PORT = argv[1]
    shell = GroundStationShell(COM_PORT)


class CliPanel(ScrollableContainer):
    """
    CLI panel class, handling command input and output displays
    """

    cli_output = reactive("")

    def __init__(self, *widgets: Widget, id: str | None = None) -> None:  # noqa: A002
        """
        Initialize the CLI panel and set up output redirection
        """
        super().__init__(*widgets, id=id)
        self.shell = shell
        self.cli_output_panel: Static | None = None
        #  Hold the original output stream
        self.sys_stdout = sys.stdout

        #  Buf creates a StringIO instance, storing what is printed from the cli through a redirection of stdout
        self.buffer = io.StringIO()

        #  Redirects the output stream (sys.stdout) to buffer, and anything printed will now be written to buffer
        sys.stdout = self.buffer

        #  Redirects the output stream of the gs shell to buffer, allowing help descriptions to be displayed
        self.shell.stdout = sys.stdout
        #  Redirects the "error" output stream of the gs shell to buffer
        sys.stderr = sys.stdout

        if self.shell.intro is not None:
            print(self.shell.intro)

    def on_mount(self) -> None:
        """
        Set up periodic CLI output refresh and initialize the output panel
        """
        self.output_refresh = self.set_interval(1 / 120, self.update_cli)
        self.cli_output_panel = self.query_one("#cli-output-panel", Static)

        # Buffer.getvalue() returns the contents of the string buffer as a str
        self.cli_output = self.buffer.getvalue()

    def on_unmount(self) -> None:
        """
        Restore original output streams when the CLI panel is unmounted
        """
        #  Upon exitting the cli, restore all original output streams
        sys.stdout = self.sys_stdout
        sys.stderr = self.sys_stdout
        self.shell.stdout = self.sys_stdout

    def watch_cli_output(self, cli_output: str) -> None:
        """
        Update the CLI output panel with the latest CLI output
        """
        if self.cli_output_panel:
            self.cli_output_panel.update(f"CLI - {COM_PORT}\n" + self.cli_output)

    def update_cli(self) -> None:
        """
        Refresh the CLI output from the buffer
        """
        self.cli_output = self.buffer.getvalue()

    #  Use threads to ensure blocking commands don't block CLI
    def run_cli_command_in_thread(self, cmd_function: Callable[[str], None], args: str) -> None:
        """
        Run a CLI command in a separate thread to avoid blocking the UI
        """
        thread = threading.Thread(target=cmd_function, args=(args,), daemon=True)
        thread.start()

    @on(Input.Submitted)
    def submit_command(self) -> None:
        """
        Handle command submission from the input widget and dispatch to the shell
        """
        cli_input = self.query_one(Input)
        print(f"(UW Orbital): {cli_input.value}")

        #  Splits the input into two parts: command name and args
        command_parts = cli_input.value.strip().split(maxsplit=1)
        try:
            # Obtain function from gs shell instance based on inputted command
            cmd_function = getattr(self.shell, f"do_{command_parts[0]}")
            args = command_parts[1] if len(command_parts) > 1 and command_parts[1] is not None else ""

            # Make special exception for "exit" command
            if command_parts[0] == "exit":
                self.app.exit()
                return

            # Change print_logs cmd button status to STOP if manually typed in
            if command_parts[0] == "print_logs":
                print("[yellow]Use print_logs button below CMDS to exit polling.[/yellow]")
                # Query_one("#btn-print_logs") returns a widget, so type cast widget to button
                btn = cast(Button, self.app.query_one("#btn-print_logs"))
                btn.label = "STOP"

            # Disable send_conn_request cmd btn if manually typed in
            if command_parts[0] == "send_conn_request":
                btn = cast(Button, self.app.query_one("#btn-send_conn_request"))
                btn.disabled = True

            self.run_cli_command_in_thread(cmd_function, args)

        except (AttributeError, IndexError):
            print(f"*** Unknown syntax: {cli_input.value}")

        self.scroll_to_bottom()
        self.query_one(Input).value = ""

    def scroll_to_bottom(self) -> None:
        """
        Auto-scroll to bottom of cli panel
        """
        self.scroll_end()

    def compose(self) -> ComposeResult:
        """
        Compose the CLI panel widgets
        """
        yield Static("", id="cli-output-panel")
        yield Input(placeholder="Enter command here:", id="cli-input")


class CmdButton(HorizontalGroup):
    """
    A horizontal group containing command buttons for the CLI
    """

    def __init__(self, cmdname: str) -> None:
        """
        Initialize the command button with the given command name
        """
        super().__init__()
        self.cmdname = cmdname
        self.shell = shell

    def on_button_pressed(self, event: Button.Pressed) -> None:
        """
        Handle the event when the command button is pressed
        """
        cmd_function = getattr(self.shell, f"do_{self.cmdname}")
        args = ""

        thread = threading.Thread(target=cmd_function, args=(args,), daemon=True)

        if event.button.label != "STOP":
            print(f"(UW Orbital): {self.cmdname}")
            thread.start()

            if self.cmdname != "print_logs":
                event.button.disabled = True

        if self.cmdname == "print_logs":
            if event.button.label == "Run":
                print("[yellow]Use print_logs button below CMDS to exit polling.[/yellow]")
                event.button.label = "STOP"
            else:
                event.button.label = "Run"
                self.shell.stop_printing = True

    def compose(self) -> ComposeResult:
        """
        Compose the command button and its label
        """
        yield Label(f"{self.cmdname}", id="button-label")
        yield Button("Run", id=f"btn-{self.cmdname}")


class TimeTaggedLogs(HorizontalScroll):
    """
    A horizontal scrollable widget displaying time-tagged command logs
    """

    def __init__(self, *widgets: Widget, id: str | None = None) -> None:  # noqa: A002
        """
        Initialize the time-tagged logs table with sample data
        """
        super().__init__(*widgets, id=id)
        self.rows = [
            ("CMD", "Time", "Cate1", "Cate2"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
            ("ABC", "ABC", "ABC", "ABC"),
        ]

    def compose(self) -> ComposeResult:
        """
        Compose the time-tagged logs table and label
        """
        yield Label("TIME TAGGED CMDS")
        yield DataTable()

    def on_mount(self) -> None:
        """
        Set up columns and rows for the time-tagged logs table
        """
        table = self.query_one(DataTable)
        table.add_columns(*self.rows[0])
        table.add_rows(self.rows[1:])


class LogsPanel(Static):
    """
    A static widget for displaying logs from a file
    """

    logs = reactive("")

    def on_mount(self) -> None:
        """
        Set up periodic log refresh for the logs panel
        """
        self.logs_refresh = self.set_interval(1 / 60, self.update_logs)

    def update_logs(self) -> None:
        """
        Read and update logs from the log file
        """
        try:
            with open("gs/backend/logs.log") as logs:
                self.logs = logs.read()
        except FileNotFoundError:
            self.update("[red]Logs file not found. Try running interface from root directory (OBC-Firmware)[/red]")

    def watch_logs(self, logs: str) -> None:
        """
        Update logs panel with the latest logs
        """
        self.update("LOGS\n\n" + self.logs)
        self.scroll_to_bottom()

    def scroll_to_bottom(self) -> None:
        """
        Auto-scroll to bottom of logs panel
        """
        scrollable_container = self.app.query_one("#logs")
        if scrollable_container:
            scrollable_container.scroll_end()


class CLIWindow(App[None]):
    """
    Main Textual application window for the CLI interface
    """

    CSS_PATH = "cli.tcss"

    def on_mount(self) -> None:
        """
        Set the theme when application mounts
        """
        self.theme = "dracula"

    def compose(self) -> ComposeResult:
        """
        Compose the main application layout with all panels and widgets
        """
        yield ScrollableContainer(LogsPanel("LOGS"), can_focus=True, id="logs")
        yield CliPanel(id="cli")
        yield VerticalScroll(
            Static("CMDS"),
            CmdButton("print_logs"),
            CmdButton("send_conn_request"),
            CmdButton("start_logging"),
            id="cmd-panel",
        )
        yield TimeTaggedLogs(id="timetagged")


def main() -> None:
    """
    Entry point for the CLI application; opens the serial port at the com port and runs the ground station cli.
    """
    if len(argv) != 2:
        print("One argument needed: Com Port")
        return

    try:
        ser = Serial(COM_PORT)
        print("Comm port set to: " + str(ser.name))
        ser.close()

    except SerialException as e:
        print(f"An error occurred while opening the serial port: {e}")

    app = CLIWindow()
    app.run()


if __name__ == "__main__":
    main()
