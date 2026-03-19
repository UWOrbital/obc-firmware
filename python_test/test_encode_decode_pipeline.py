import random

from ax25 import FrameType
from interfaces import (
    CUBE_SAT_CALLSIGN,
    GROUND_STATION_CALLSIGN,
    INFO_FIELD_END_POSITION,
    INFO_FIELD_START_POSITION,
    RS_DECODED_DATA_SIZE,
)
from interfaces.obc_gs_interface.aes128 import AES128
from interfaces.obc_gs_interface.ax25 import AX25
from interfaces.obc_gs_interface.commands.python import (
    CmdCallbackId,
    pack_command,
    unpack_command,
)
from interfaces.obc_gs_interface.commands.python.command_factories import (
    create_cmd_ping,
    create_cmd_rtc_sync,
)
from interfaces.obc_gs_interface.fec import FEC


# TEST: A simulated receive with the entire pipline
def test_receive():
    # Data from the c tests
    c_data = [
        0x7E,
        0x82,
        0x96,
        0x92,
        0xA8,
        0x9E,
        0x40,
        0x60,
        0x82,
        0xA8,
        0x98,
        0x82,
        0xA6,
        0x40,
        0x61,
        0x0,
        0xF0,
        0x83,
        0x2B,
        0x3E,
        0x24,
        0x1C,
        0xB,
        0x5A,
        0x85,
        0x21,
        0x38,
        0xB4,
        0xF1,
        0xF1,
        0xC3,
        0x47,
        0x32,
        0xC0,
        0x3E,
        0x22,
        0xD5,
        0xAA,
        0x8,
        0xA4,
        0x10,
        0x71,
        0x8,
        0x8B,
        0x68,
        0xC1,
        0x9A,
        0x3A,
        0xD,
        0x2F,
        0xF,
        0x71,
        0x96,
        0xCD,
        0xF7,
        0x32,
        0x0,
        0x86,
        0x86,
        0xA3,
        0x63,
        0xB,
        0xED,
        0x2,
        0x20,
        0x60,
        0x31,
        0xF4,
        0xAF,
        0x4F,
        0xE,
        0xD6,
        0xEB,
        0x8C,
        0x14,
        0xC2,
        0x69,
        0x9,
        0xAF,
        0xBA,
        0x46,
        0x39,
        0xA0,
        0x7A,
        0x37,
        0x2F,
        0x95,
        0x69,
        0x8,
        0x1F,
        0x29,
        0xF1,
        0xEE,
        0xAD,
        0x6,
        0x65,
        0x63,
        0x21,
        0x40,
        0xBA,
        0xC9,
        0x22,
        0x30,
        0xE3,
        0xD4,
        0x21,
        0x1B,
        0xE5,
        0x16,
        0xB4,
        0x9,
        0x99,
        0x18,
        0xB,
        0x7C,
        0xA4,
        0xA9,
        0x58,
        0x9,
        0xA9,
        0x0,
        0x1B,
        0x5B,
        0xD9,
        0x98,
        0x36,
        0x5F,
        0x70,
        0x97,
        0x20,
        0x2F,
        0xA4,
        0xAC,
        0xCB,
        0x2,
        0xA5,
        0xB7,
        0x20,
        0x2E,
        0x72,
        0xD2,
        0x8F,
        0x1E,
        0x91,
        0x8B,
        0x73,
        0x68,
        0xD3,
        0xD2,
        0x8,
        0x47,
        0xCD,
        0xD3,
        0xAE,
        0x36,
        0x14,
        0xF,
        0x86,
        0x9B,
        0x25,
        0x20,
        0x98,
        0x23,
        0x40,
        0x4C,
        0x83,
        0xC,
        0xB9,
        0x3A,
        0xE0,
        0xD1,
        0x7D,
        0xD5,
        0x1E,
        0x18,
        0x14,
        0xA9,
        0x47,
        0x3C,
        0x2F,
        0x5D,
        0x37,
        0xC3,
        0x2E,
        0xBE,
        0x95,
        0x1,
        0x7C,
        0x37,
        0x61,
        0x60,
        0xEB,
        0xE9,
        0x4C,
        0xCD,
        0x6,
        0xEF,
        0x98,
        0x3E,
        0xA,
        0xDE,
        0xCC,
        0xC5,
        0xE9,
        0xF1,
        0x98,
        0xAC,
        0xD2,
        0x4,
        0x83,
        0x95,
        0x3A,
        0x9C,
        0xB,
        0xEC,
        0xA8,
        0xBA,
        0x73,
        0x11,
        0xF5,
        0xDF,
        0x8F,
        0x1,
        0xCF,
        0x60,
        0x2B,
        0xE3,
        0x2E,
        0xE1,
        0x62,
        0x79,
        0x2B,
        0x6C,
        0xA6,
        0x2F,
        0x6E,
        0x1F,
        0x5F,
        0x55,
        0xAB,
        0x49,
        0xF3,
        0x14,
        0x30,
        0x73,
        0x72,
        0x45,
        0x30,
        0xFA,
        0x49,
        0xE4,
        0x89,
        0x2D,
        0x75,
        0x50,
        0x43,
        0x59,
        0x53,
        0xE7,
        0xCE,
        0xF2,
        0x7,
        0xDC,
        0x2C,
        0x73,
        0x7A,
        0x31,
        0x7C,
        0x3E,
        0xF8,
        0xCA,
        0x25,
        0xE7,
        0x28,
        0x15,
        0x31,
        0x22,
        0x9,
        0xC8,
        0x7E,
    ]
    bin = bytearray(c_data)

    # Instantiate the ax25 class to unstuff
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    bin = ax25_proto.unstuff(bytes(bin))

    # Instantiate FEC class to error correct
    fec_coder = FEC()
    # NOTE: 17 (inclusive) to 272 (exclusive) is the range for info bytes that are needed for the decoding
    data_to_decode = fec_coder.decode(bin[INFO_FIELD_START_POSITION : INFO_FIELD_END_POSITION + 1])
    # With the data decoded we need to add the rest of the data back to get a full frame
    decoded_data = bytes(bin[:INFO_FIELD_START_POSITION] + data_to_decode + bin[INFO_FIELD_END_POSITION + 1 :])

    # Now we can finally decode the frame and extract information
    rcv_frame = ax25_proto.decode_frame(decoded_data)
    assert str(rcv_frame.src) == GROUND_STATION_CALLSIGN
    assert str(rcv_frame.dst) == CUBE_SAT_CALLSIGN
    assert str(rcv_frame.control.frame_type) == "FrameType.I"
    frame_data = rcv_frame.data

    # Instantiate the aes class with defaults from the c implementation and decrypt the data
    aes_cipher = AES128(
        b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
        b"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
    )
    decrypted_data = b""

    if frame_data is not None:
        decrypted_data = aes_cipher.decrypt(bytes(frame_data))

    # What the data should be and see if it matches
    intended_str = "64 121 190 31 108 53 202 59 88 177 150 23 4 237 34 179 112 233 110 15 156 165 122 43 136 33 70 7 52 93 210 163 160 89 30 255 204 21 42 27 184 145 246 247 100 205 130 147 208 201 206 239 252 133 218 11 232 1 166 231 148 61 50 131 0 57 126 223 44 245 138 251 24 113 86 215 196 173 226 115 48 169 46 207 92 101 58 235 72 225 6 199 244 29 146 99 96 25 222 191 140 213 234 219 120 81 182 183 36 141 66 83 144 137 142 175 188 69 154 203 168 193 102 167 84 253 242 67 192 249 62 159 236 181 74 187 216 49 22 151 132 109 162 51 240 105 238 143 28 37 250 171 8 161 198 135 180 221 82 35 32 217 158 127 76 149 170 155 56 17 118 119 228 77 2 19 80 73 78 111 124 5 90 139 104 129 38 103 20 189 178 3 128 185 254 95 172 117 10 123 152 241 214 87 68 45 98 243 176 41 174 79 220 229 186 107 200 97 134 71 116 157 18 "
    actual_str = ""
    for i in range(RS_DECODED_DATA_SIZE):
        actual_str += str(decrypted_data[i]) + " "

    assert intended_str == actual_str


# TEST: A simulated send with the entire pipline (some bits are also flipped before sending to test fec)
def test_send():
    # Instantiate our ax25 class to get ready to create frame
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    # Instantiate the fec class for forward error correction
    fec_coder = FEC()

    # Generating pseudo-random data
    data = []
    random.seed(777)
    for _ in range(RS_DECODED_DATA_SIZE):
        data.append(random.randint(0, 255))

    # Instantaite the aes cipher with the same defaults from the c implementation
    aes_cipher = AES128(
        b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
        b"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
    )
    # Encrypt data
    encrypted_data = aes_cipher.encrypt(bytes(data))
    # Encode data for error correction
    encode_data = fec_coder.encode(bytes(encrypted_data))
    # Create the frame
    send_frame = ax25_proto.encode_frame(encode_data, FrameType.I, 0)
    # Stuff the frame as per the standard
    send_frame_stuffed = ax25_proto.stuff(send_frame)
    # Convert to byte array so we can change data
    frame_array = bytearray(send_frame_stuffed)

    frame_array[18] += 1
    frame_array[200] += 1

    # Convert back to bytes
    send_frame = bytes(frame_array)

    # Print this out to console so they can be copied to the C tests
    # NOTE: To see this output use the -s flag with pytest (pytest -s)
    print([hex(byte) for byte in send_frame])
    print(len(send_frame))

    # If it gets here that means all went well!
    assert True


def test_uFrame_send():
    # Instantiate our ax25 class to get ready to create frame
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    # Create the frame
    send_frame = ax25_proto.encode_frame(None, FrameType.SABM, 0, True)
    send_frame = ax25_proto.stuff(send_frame)
    # NOTE: To see this output use the -s flag with pytest (pytest -s)
    print([hex(byte) for byte in send_frame])


def test_uFrame_receive():
    output_from_c = [
        "0x7e",
        "0x82",
        "0x96",
        "0x92",
        "0xa8",
        "0x9e",
        "0x40",
        "0x60",
        "0x82",
        "0xa8",
        "0x98",
        "0x82",
        "0xa6",
        "0x40",
        "0x61",
        "0x3e",
        "0xcd",
        "0x3d",
        "0x80",
        "0x7e",
    ]
    receive_bytes = bytes([int(x, 0) for x in output_from_c])
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    # Unstuff frame
    rcv_frame = ax25_proto.unstuff(receive_bytes)
    # Create the frame
    rcv_frame = ax25_proto.decode_frame(rcv_frame)

    assert rcv_frame.control.frame_type == FrameType.SABM
    assert rcv_frame.control.poll_final == True


def test_command_send():
    # Instantiate our ax25 class to get ready to create frame
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    # Instantiate the fec class for forward error correction
    fec_coder = FEC()

    # Let's generate some packed commands here: A ping command and a RTC Sync Command
    cmd_ping = create_cmd_ping()
    cmd_rtc_sync = create_cmd_rtc_sync(1234567)
    cmd_ping_packed = bytearray(pack_command(cmd_ping))
    cmd_rtc_sync_packed = bytearray(pack_command(cmd_rtc_sync))

    data = bytes(cmd_ping_packed + cmd_rtc_sync_packed).ljust(RS_DECODED_DATA_SIZE, b"\x00")
    print(data)
    # Instantaite the aes cipher with the same defaults from the c implementation
    aes_cipher = AES128(
        b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
        b"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
    )
    # Encrypt data
    encrypted_data = aes_cipher.encrypt(data)
    # Encode data for error correction
    encode_data = fec_coder.encode(bytes(encrypted_data))
    # Create the frame
    send_frame = ax25_proto.encode_frame(encode_data, FrameType.I, 0)
    # Stuff the frame as per the standard
    send_frame_stuffed = ax25_proto.stuff(send_frame)
    # Convert to byte array so we can change data
    frame_array = bytearray(send_frame_stuffed)

    # Convert back to bytes
    send_frame = bytes(frame_array)

    # Print this out to console so they can be copied to the C tests
    # NOTE: To see this output use the -s flag with pytest (pytest -s)
    print([hex(byte) for byte in send_frame])
    print(len(send_frame))

    # If it gets here that means all went well!
    assert True


def test_receive_command():
    # Data from the c tests
    c_data = [
        0x7E,
        0x82,
        0x96,
        0x92,
        0xA8,
        0x9E,
        0x40,
        0x60,
        0x82,
        0xA8,
        0x98,
        0x82,
        0xA6,
        0x40,
        0x61,
        0x20,
        0xF0,
        0x1,
        0x0,
        0x0,
        0x0,
        0x0,
        0x5,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x81,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0xA3,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x3C,
        0xE7,
        0x73,
        0x4A,
        0xC4,
        0xF9,
        0x4,
        0xD,
        0xD0,
        0xE,
        0xEE,
        0xCA,
        0xE,
        0x46,
        0x8E,
        0xF9,
        0x62,
        0xFA,
        0xA9,
        0xC9,
        0xAE,
        0x82,
        0x6D,
        0xA,
        0x25,
        0x2F,
        0x4C,
        0xA3,
        0x6A,
        0x22,
        0x13,
        0x1,
        0x40,
        0xF0,
        0xA0,
        0x7E,
    ]

    bin = bytearray(c_data)

    # Instantiate the ax25 class to unstuff
    ax25_proto = AX25(GROUND_STATION_CALLSIGN, CUBE_SAT_CALLSIGN)
    bin = ax25_proto.unstuff(bytes(bin))

    # Instantiate FEC class to error correct
    fec_coder = FEC()
    # NOTE: 17 (inclusive) to 272 (exclusive) is the range for info bytes that are needed for the decoding
    data_to_decode = fec_coder.decode(bin[INFO_FIELD_START_POSITION : INFO_FIELD_END_POSITION + 1])
    # With the data decoded we need to add the rest of the data back to get a full frame
    decoded_data = bytes(bin[:INFO_FIELD_START_POSITION] + data_to_decode + bin[INFO_FIELD_END_POSITION + 1 :])

    # Now we can finally decode the frame and extract information
    rcv_frame = ax25_proto.decode_frame(decoded_data)
    assert str(rcv_frame.src) == GROUND_STATION_CALLSIGN
    assert str(rcv_frame.dst) == CUBE_SAT_CALLSIGN
    assert str(rcv_frame.control.frame_type) == "FrameType.I"
    frame_data = rcv_frame.data

    cmd_list = []

    # NOTE: unpack_command returns a tuple with the first element having a list of command messages and the second having the bytes that could not be unpacked
    if frame_data is not None:
        cmd_list = unpack_command(bytes(frame_data[:223]))[0]

    match_cmd = [CmdCallbackId.CMD_EXEC_OBC_RESET.value, CmdCallbackId.CMD_PING.value]

    # See if the cmds we got back match
    for i in range(2):
        assert cmd_list[i].id == match_cmd[i]

    # NOTE: This must be called after every test is ran to avoid Segfaulting (Thus, it is not called in the first few tests)
    fec_coder.destroy()
