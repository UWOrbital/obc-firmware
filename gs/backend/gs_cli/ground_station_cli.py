from cmd import Cmd
from datetime import datetime
from multiprocessing import Process
from sys import argv, exit

from serial import Serial, SerialException

from gs.backend.obc_utils.command_utils import LOG_PATH, arg_parse, poll, send_command, send_conn_request
from interfaces.obc_gs_interface.commands import CmdCallbackId


class GroundStationShell(Cmd):
    """
    Cmd Class for the Wired Comms Command Line Interface
    """

    def __init__(self, com_port: str) -> None:
        Cmd.__init__(self)
        self.parser = arg_parse()
        self._com_port: str = com_port
        self._conn_request_sent: bool = False
        self.background_logging: Process | None = None
        self.stop_printing = False

        # At the start of each command shell use, we clear the file and create a dated title
        with open(LOG_PATH, "w") as file:
            file.write("LOGS (Date: " + str(datetime.now()) + ")\n")

    intro = """

    ██╗   ██╗██╗    ██╗     ██████╗ ██████╗ ██████╗ ██╗████████╗ █████╗ ██╗
    ██║   ██║██║    ██║    ██╔═══██╗██╔══██╗██╔══██╗██║╚══██╔══╝██╔══██╗██║
    ██║   ██║██║ █╗ ██║    ██║   ██║██████╔╝██████╔╝██║   ██║   ███████║██║
    ██║   ██║██║███╗██║    ██║   ██║██╔══██╗██╔══██╗██║   ██║   ██╔══██║██║
    ╚██████╔╝╚███╔███╔╝    ╚██████╔╝██║  ██║██████╔╝██║   ██║   ██║  ██║███████╗
     ╚═════╝  ╚══╝╚══╝      ╚═════╝ ╚═╝  ╚═╝╚═════╝ ╚═╝   ╚═╝   ╚═╝  ╚═╝╚══════╝
  Welcome to the UW Orbital Command Line Interface! Type help or ? to list commands.\n
    """
    prompt = "(UW Orbital): "
    file = None

    def _restart_logging(self) -> None:
        """
        A function with the appropriate checks to restart the logging process
        """
        if self.background_logging is not None and not self.background_logging.is_alive():
            self.background_logging = Process(
                target=poll,
                args=(
                    self._com_port,
                    LOG_PATH,
                    False,
                ),
                daemon=True,
            )
            self.background_logging.start()

    def do_send_conn_request(self, line: str) -> None:
        "Sends the initial connection request"
        # Preliminary checks for the function to run
        if self._conn_request_sent:
            print("Connection Request has already been sent. Aborting...")
            return

        # We try to send a connection request and if no response is recieved we catch the indexError and handle it
        try:
            if self.background_logging is not None:
                self.background_logging.kill()
            send_conn_request(self._com_port, 1)
        except IndexError:
            print("Connection request was not successful. Try resetting the board")
            return
        finally:
            self._restart_logging()

        print("Connection request successfully sent.")
        self._conn_request_sent = True

    def do_send_command(self, line: str) -> None:
        """
        Sends a command to the ground station
        """
        # Preliminary checks for the function to run
        if not self._conn_request_sent:
            print("Connection Request needs to be sent first. Aborting...")
            return

        if self.background_logging is not None:
            self.background_logging.kill()

        cmd_response = send_command(line, self._com_port, 1)
        print(cmd_response)
        if cmd_response is not None and cmd_response.cmd_id == CmdCallbackId.CMD_EXEC_OBC_RESET:
            self._conn_request_sent = False

        self._restart_logging()

    def do_set_comm_port(self, line: str) -> None:
        """
        Sets the comm port to be used to send commands
        """
        # Here we try to open the serial port to see if it is valid, if not we catch the SerialException and handle it
        try:
            ser = Serial(line)
            print("Comm port set to: " + str(ser.name))
            ser.close()
            self._com_port = line
        except SerialException:
            print("Invalid port entered")
            return

    def do_start_logging(self, line: str) -> None:
        """
        Start writing logs from the board in a file
        """
        # Here we have to be careful that everything is configured and that we don't start two processes, thus the
        # checks
        if self.background_logging is None or not self.background_logging.is_alive():
            self.background_logging = Process(
                target=poll,
                args=(
                    self._com_port,
                    LOG_PATH,
                    False,
                ),
                daemon=True,
            )
            self.background_logging.start()
        else:
            print("Logging has already been started")

    def do_print_logs(self, line: str) -> None:
        """
        Prints out logs and polls for log_pathlogs that are coming in. Use a Keyboard Interrupt to exit (e.g. Ctrl + C)
        """
        self.stop_printing = False

        # Preliminary checks for the function to run
        # Write out the logs that we previously got
        with open(LOG_PATH) as file:
            print("Printing " + file.read())

        if self.background_logging is not None:
            self.background_logging.kill()

        # Here we run the function and catch an interrupt if it is executed by the user
        try:
            # We use lambda so that the poll function can acquire updated stop_printing values later
            poll(self._com_port, LOG_PATH, 1, lambda: self.stop_printing)
        except KeyboardInterrupt:
            print("Exiting polling...")
        finally:
            print("Exiting polling...")

        self._restart_logging()

    def help_send_command(self) -> None:
        """
        Help to send a command
        """
        self.parser.print_help()

    def do_exit(self, arg: str) -> None | bool:
        "Close the UW Orbital CLI"
        print("Closing CLI...")
        exit()


def main() -> None:
    """
    A function that initializes the com port and starts the ground station shell
    """
    if len(argv) != 2:
        print("One argument needed: Com Port")
        return

    try:
        com_port = str(argv[1])
        ser = Serial(com_port)
        print("Comm port set to: " + str(ser.name))
        ser.close()
        GroundStationShell(com_port).cmdloop()
    except SerialException:
        print("Invalid port entered")
        # We call a loop and the command shell runs as expected


if __name__ == "__main__":
    main()
