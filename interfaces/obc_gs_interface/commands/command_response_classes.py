from dataclasses import dataclass

from interfaces.obc_gs_interface.commands import CmdCallbackId, CmdResponseErrorCode


@dataclass
class CmdRes:
    """
    A parent class for all responses

    :param cmd_id: The id of the command that was sent
    :type cmd_id: CmdCallbackId
    :param error_code: Stores whether the command was successfully executed or if there was an error
    :type error_code: CmdResponseErrorCode
    """

    cmd_id: CmdCallbackId
    error_code: CmdResponseErrorCode
    response_length: int

    def __str__(self) -> str:
        """
        Overriding the str method for a better representation of what's happening
        """
        formatted_string = ""
        formatted_string += "Class Name: CmdRes\n"
        formatted_string += "Cmd ID: " + self.cmd_id.name + "\n"
        if self.error_code == CmdResponseErrorCode.CMD_RESPONSE_SUCCESS:
            formatted_string += "Command Execution: SUCCESS!\n"
        else:
            formatted_string += "Command Execution: ERROR!\n"

        formatted_string += "Response Length: " + str(self.response_length) + "\n"

        return formatted_string


@dataclass
class CmdRtcSyncRes(CmdRes):
    """
    Child class for storing the response to the CMD_RTC_SYNC

    :param board_unixtime: The time on the board when the sync command was sent
    :type board_unixtime: int
    """

    board_unixtime: int

    def __str__(self) -> str:
        """
        Overriding the str method for a better representation of what's happening
        """
        formatted_string = super().__str__()
        formatted_string += "Unixtime from Board: " + str(self.board_unixtime) + "\n"

        return formatted_string


@dataclass
class CmdVerifyCrcRes(CmdRes):
    """
    Class for storing the response to CMD_VERIFY_CRC

    :param crc: The crc of the application stored on the board
    :type crc: int
    """

    crc: int

    def __str__(self) -> str:
        """
        Overriding the str method for a better representation of what's happening
        """
        formatted_string = super().__str__()
        formatted_string += "Application CRC: " + str(self.crc) + "\n"

        return formatted_string


@dataclass
class CmdI2CProbeRes(CmdRes):
    """
    Class for storing the response to CMD_I2C_PROBE

    :param valid_addresses: The valid I2C addresses from the board
    :type valid_addresses: list[int]
    """

    valid_addresses: list[int]

    def __str__(self) -> str:
        """
        Overriding the str method for a better representation of what's happening
        """
        formatted_string = super().__str__()
        formatted_string += "Valid I2C Addresses: |"
        for address in self.valid_addresses:
            formatted_string += " " + str(address) + " |"

        return formatted_string


if __name__ == "__main__":
    cmd = CmdVerifyCrcRes(CmdCallbackId.CMD_VERIFY_CRC, CmdResponseErrorCode.CMD_RESPONSE_ERROR, 4, 0x12345678)
    print(cmd)
