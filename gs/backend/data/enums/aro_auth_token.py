from enum import StrEnum, auto


class AROAuthToken(StrEnum):
    """
    The possible states that an AROAuthToken can be
    CURRENTLY DUMMY STATES
    """

    # TODO Implmement proper states
    DUMMY = auto()  # Command was created in the db but not yet sent to the OBC
    ANOTHERDUMMY = auto()  # Command was sent to OBC
    TEST = auto()  # Command is executing on OBC
