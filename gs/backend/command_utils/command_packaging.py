from ax25 import Frame, FrameType

from interfaces import (
    CUBE_SAT_CALLSIGN,
    GROUND_STATION_CALLSIGN,
    INFO_FIELD_END_POSITION,
    INFO_FIELD_START_POSITION,
    RS_DECODED_DATA_SIZE,
    RS_ENCODED_DATA_SIZE,
)
from interfaces.obc_gs_interface.aes128 import AES128
from interfaces.obc_gs_interface.ax25 import AX25
from interfaces.obc_gs_interface.fec import FEC


class CommandPackaging:
    """
    A Class that stores functions to encode/decode data from the Comms Pipeline
    """

    def __init__(
        self,
        src: str = GROUND_STATION_CALLSIGN,
        dst: str = CUBE_SAT_CALLSIGN,
        aes_key: bytes = b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
        aes_iv: bytes = b"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
    ) -> None:
        self._ax25 = AX25(src, dst)
        self._fec = FEC()
        self._aes = AES128(aes_key, aes_iv)
        # TODO: AES is hard-coded, this will need to be changed. Additionally the iv will need to be framed

    def encode_frame(self, data: None | bytes) -> bytes:
        """
        Given data that is less than 223 bytes, this function applies fec and aes128 and frames the data in
        accordance with the ax25 protocol with bit stuffing

        :param data: The data to be encoded
        :return: The function returns the encoded bytes. Note, these bytes are stuffed
        """
        # No data is assumed to be a control frame
        if data is None:
            # Create the frame
            send_frame = self._ax25.encode_frame(None, FrameType.SABM, 0, True)
        else:
            # If data is provided, it needs to be less than the RS_DECODED_DATA_SIZE
            if len(data) > RS_DECODED_DATA_SIZE:
                raise ValueError(f"Invalid Argument: Data to encode is too long, must <= {RS_DECODED_DATA_SIZE} bytes")
            # Encrypt data
            encrypted_data = self._aes.encrypt(data)
            # Encode data for error correction
            encode_data = self._fec.encode(bytes(encrypted_data))
            # Create the frame
            send_frame = self._ax25.encode_frame(encode_data, FrameType.I, 0)

        # Stuff the frame as per the standard
        send_frame_stuffed = self._ax25.stuff(send_frame)

        return send_frame_stuffed

    def decode_frame(self, data: bytes) -> Frame | None:
        """
        Given a stuffed ax25 frame with fec and aes128, this function decodes the data into a usable Frame object from
        the pyham_ax25 library

        :param data: Bytes containing data with fec, aes128 and ax25 framing with bit stuffing
        :return: A Frame object representing that data
        """
        if len(data) == 0:
            return None
        elif len(data) > RS_ENCODED_DATA_SIZE:
            # Unstuff the data
            data_unstuffed = self._ax25.unstuff(bytes(data))
            # NOTE: 17 (inclusive) to 272 (exclusive) is the range for info bytes that are needed for the decoding
            fec_data = self._fec.decode(data_unstuffed[INFO_FIELD_START_POSITION : INFO_FIELD_END_POSITION + 1])
            # With the data decoded we need to add the rest of the data back to get a full frame
            decoded_data = bytes(
                data_unstuffed[:INFO_FIELD_START_POSITION] + fec_data + data_unstuffed[INFO_FIELD_END_POSITION + 1 :]
            )
            # Turn the bytes into the frame using the ax25 library
            rcv_frame = self._ax25.decode_frame(decoded_data)

        else:
            rcv_frame = self._ax25.decode_frame(data)

        return rcv_frame
