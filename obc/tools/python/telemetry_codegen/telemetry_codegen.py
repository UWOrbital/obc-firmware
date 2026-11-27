import re
from typing import TypedDict, cast

import toml


class _Signal(TypedDict):
    name: str
    bit_width: int
    min_value: int
    max_value: int
    value_type: str


class _Message(TypedDict):
    id: int
    name: str
    signals: list[_Signal]


class _Data(TypedDict):
    messages: list[_Message]


def _generate_c_macros(messages: list[_Message]) -> str:
    c_code = []
    reg = re.compile(r"^[_a-z][0-9_a-z]+$", re.IGNORECASE)

    for message in messages:
        if reg.match(message["name"]) is None:
            raise Exception("Invalid message name")

        c_code.append(f'// {message["name"]}\n')
        c_code.append(f'#define telemetry_{message["name"]}_msg_id {message["id"]}\n')

        for signal in message["signals"]:
            if reg.match(signal["name"]) is None:
                raise Exception("Invalid signal name")
            c_code.append(f'#define telemetry_{message["name"]}_{signal["name"]}_max {signal["max_value"]}\n')
            c_code.append(f'#define telemetry_{message["name"]}_{signal["name"]}_min {signal["min_value"]}\n')

        c_code.append("\n")

    return "".join(c_code)


def _generate_c_code_msg_defs(messages: list[_Message]) -> str:
    c_code = []

    for message in messages:
        c_code.append("typedef struct {{\n")
        for signal in message["signals"]:
            if signal["bit_width"] <= 0:
                raise Exception("Signal bit width is too small")
            elif signal["bit_width"] <= 8:
                datatype = "uint8_t"
            elif signal["bit_width"] <= 16:
                datatype = "uint16_t"
            elif signal["bit_width"] <= 32:
                datatype = "uint32_t"
            elif signal["bit_width"] <= 64:
                datatype = "uint64_t"
            else:
                raise Exception("Signal bit width is too large")
            c_code.append(f'    {datatype} {signal["name"]}; // {signal["bit_width"]} bits\n')
        c_code.append(f'}} telemetry_{message["name"]}_msg_t;\n\n')

    return "".join(c_code)


def _generate_c_code_msg_mailboxes(messages: list[_Message]) -> str:
    c_code = []

    for message in messages:
        c_code.append(f'static telemetry_{message["name"]}_msg_t {message["name"]}_mailbox = {{0}};\n')

    c_code.append("\n")

    return "".join(c_code)


def _generate_c_set_get_functions(messages: list[_Message]) -> str:
    c_code = []

    for message in messages:
        for signal in message["signals"]:
            max_value = 2 ** signal["bit_width"] - 1
            if signal["max_value"] < signal["min_value"]:
                raise Exception("Invalid min & max values")
            range_value = signal["max_value"] - signal["min_value"]

            # Determine the smallest integer type for the signal
            if signal["bit_width"] <= 8:
                datatype = "uint8_t"
            elif signal["bit_width"] <= 16:
                datatype = "uint16_t"
            elif signal["bit_width"] <= 32:
                datatype = "uint32_t"
            else:
                datatype = "uint64_t"

            if signal["value_type"] == "continuous":
                set_func = (
                    f'void telemetry_set_{message["name"]}_{signal["name"]}(float value) {{\n'
                    f'    value = (value - {signal["min_value"]}) / {range_value} * {hex(max_value)};\n'
                    f'    {message["name"]}_mailbox.{signal["name"]} = ({datatype})(value + 0.5);\n'
                    f'}}\n\n'
                )
                get_func = (
                    f'float telemetry_get_{message["name"]}_{signal["name"]}(void) {{\n'
                    f'    {datatype} value = {message["name"]}_mailbox.{signal["name"]};\n'
                    f'    return ((float)value / {hex(max_value)} * {range_value}) + {signal["min_value"]};\n'
                    f'}}\n\n'
                )
            else:  # discrete
                set_func = (
                    f'void telemetry_set_{message["name"]}_{signal["name"]}({datatype} value) {{\n'
                    f'    {message["name"]}_mailbox.{signal["name"]} = value & {hex(max_value)};\n'
                    f'}}\n\n'
                )
                get_func = (
                    f'{datatype} telemetry_get_{message["name"]}_{signal["name"]}(void) {{\n'
                    f'    return {message["name"]}_mailbox.{signal["name"]};\n'
                    f'}}\n\n'
                )

            c_code.append(set_func)
            c_code.append(get_func)

    return "".join(c_code)


def _generate_c_serialization_deserialization_functions(messages: list[_Message]) -> str:
    c_code = []

    for message in messages:
        total_bits = sum(signal["bit_width"] for signal in message["signals"])
        total_bytes = (total_bits + 7) // 8  # Total bytes for serialized message, with padding

        # Serialization function
        serialize_func = (
            f'void serialize_{message["name"]}(uint8_t* buffer) {{\n'
            f'    uint64_t bit_stream = 0U;\n'
            f'    uint32_t bit_offset = 0U;\n'
        )

        # Loop through each signal for serialization
        for signal in message["signals"]:
            serialize_func += (
                f'    bit_stream |= ((uint64_t){message["name"]}_mailbox.{signal["name"]} & '
                f'{hex(2**signal["bit_width"] - 1)}) << bit_offset;\n'
                f'    bit_offset += {signal["bit_width"]};\n'
            )

        # Add padding bits (if necessary) and convert bit stream to byte array
        serialize_func += (
            f"    for (int i = 0; i < {total_bytes}; ++i) {{\n"
            f"        buffer[i] = (bit_stream >> (8 * i)) & 0xFF;\n"
            f"    }}\n"
            f"}}\n\n"
        )
        c_code.append(serialize_func)

        # Deserialization function
        deserialize_func = (
            f'void deserialize_{message["name"]}(uint8_t* buffer) {{\n'
            f'    uint64_t bit_stream = 0U;\n'
            f'    uint32_t bit_offset = 0U;\n'
            f'    for (int i = 0; i < {total_bytes}; ++i) {{\n'
            f'        bit_stream |= ((uint64_t)buffer[i]) << (8 * i);\n'
            f'    }}\n'
        )

        # Loop through each signal for deserialization
        for signal in message["signals"]:
            deserialize_func += (
                f'    {message["name"]}_mailbox.{signal["name"]} = '
                f'(bit_stream >> bit_offset) & {hex(2**signal["bit_width"] - 1)};\n'
                f'    bit_offset += {signal["bit_width"]};\n'
            )
        deserialize_func += "}}\n\n"
        c_code.append(deserialize_func)

    return "".join(c_code)


def generate_c_code_with_struct_and_macros(data: _Data) -> str:
    """Generate C code with advanced serialization, mailbox struct, and min/max macros."""
    c_code = ["#pragma once\n\n#include <stdint.h>\n\n"]

    c_code.append(_generate_c_macros(data["messages"]))
    c_code.append(_generate_c_code_msg_defs(data["messages"]))
    c_code.append(_generate_c_code_msg_mailboxes(data["messages"]))
    c_code.append(_generate_c_set_get_functions(data["messages"]))
    c_code.append(_generate_c_serialization_deserialization_functions(data["messages"]))

    return "".join(c_code)


# Generate the C code with advanced serialization for the non-standard TOML data
with open("telemetry_input.toml") as f:
    c_module_code_advanced_serialization = generate_c_code_with_struct_and_macros(cast(_Data, toml.load(f)))

with open("telem.h", "w") as f:
    f.write(c_module_code_advanced_serialization)
