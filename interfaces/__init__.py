from typing import Final

# NOTE: This is a list of commonly used constants throughout the python implementations of C code
MAX_CMD_MSG_SIZE: Final[int] = 16
MAX_REPONSE_PACKED_SIZE: Final[int] = 16
RS_DECODED_DATA_SIZE: Final[int] = 223
RS_ENCODED_DATA_SIZE: Final[int] = 255
OBC_UART_BAUD_RATE: Final = 115200
INFO_FIELD_START_POSITION: Final[int] = 17
INFO_FIELD_END_POSITION: Final[int] = 271
AX25_NON_INFO_BYTES: Final[int] = 18

GROUND_STATION_CALLSIGN: Final[str] = "ATLAS"
CUBE_SAT_CALLSIGN: Final[str] = "AKITO"

# This is a constant value set in the python and OBC side as to what length of I Frame the OBC will be waiting to
# receive. This must be followed or the obc will not function as expected
PADDING_REQUIRED: Final[int] = 300
