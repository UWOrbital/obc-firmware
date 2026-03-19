import warnings

from gs.backend.command_utils.command_packaging import CommandPackaging
from gs.backend.data.resources.cli_commands import CLICommand
from interfaces import PADDING_REQUIRED
from interfaces.obc_gs_interface.commands.python import CmdMsg
from interfaces.obc_gs_interface.commands.python.command_framing import command_multi_pack


class CommandsPipeline:
    """
    Recieves, sorts, and packets commands such that they may be sent to the
    satellite.
    """

    def __init__(self) -> None:
        """
        Lockout should be set at some arbitrary time before session begins.
        Once lockout is True, commands will no longer be recieved
        """
        self.lockout: bool = False
        self.commands_queue: list[CLICommand] = []
        self.packet_list: list[bytes] = []

    def queue_to_packet(self) -> None:
        """
        Converts all commands in the queue into packets.
        """
        command_messages: list[CmdMsg] = []
        command_bytes: list[bytes] = []
        comms = CommandPackaging()

        for command in self.commands_queue:
            command_messages.append(command.cmd_msg)

        command_bytes = command_multi_pack(command_messages)

        for byte_string in command_bytes:
            self.packet_list.append(comms.encode_frame(byte_string).ljust(PADDING_REQUIRED, b"\x00"))

    def add_to_queue(self, command: CLICommand) -> tuple[bool, list[CLICommand]]:
        """
        Inserts a command into queue, and then sorts it by priority and then by time.


        :command: CLICommand which has been rehydrated with the relevant infromation
        :return: tuple containing a status which is True if command has been inserted and
                 False otherwise and the command queue
        """

        if self.lockout:
            warnings.warn("Commands queue lockout active. Returned current queue", stacklevel=2)
            return False, self.commands_queue

        self.commands_queue.append(command)
        self.sort_queue()

        return True, self.commands_queue

    def sort_queue(self) -> list[CLICommand]:
        """
        This function sorts the queue 2 times. We first sort by time to ensure time descending,
        then we sort by priority to ensure that the highest priority is at the top of the
        queue.
        """
        self.commands_queue.sort(key=lambda x: x.time)
        self.commands_queue.sort(key=lambda x: x.prio)
        return self.commands_queue

    def apply_rule(self) -> list[CLICommand]:
        """
        There are certain rules which commands must follow.
        For example, some commands CANNOT come before other commands.
        These rules can be defined and applied to the current queue
        """
        return self.commands_queue

    def clear_queue(self) -> list[CLICommand]:
        """
        Clears the current queue
        """
        self.commands_queue = []
        return self.commands_queue

    def enable_lockout(self) -> None:
        """
        Prevents more commands from being recieved.
        """
        self.lockout = True

    def disable_lockout(self) -> None:
        """
        Allows more commands to be recieved
        """
        self.lockout = False
