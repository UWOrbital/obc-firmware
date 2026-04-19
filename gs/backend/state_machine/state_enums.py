from enum import StrEnum, auto


class StateMachineStates(StrEnum):
    """
    The states of the ground station state machine
    Defaults to disconnected
    """

    # UPLINK STATES
    DISCONNECTED = auto()
    ATTEMPTING_CONNECTION = auto()
    AWAITING_ACK = auto()
    UPLINKING = auto()
    DOWNLINKING = auto()

    # DISCONNECT STATES
    AWAITING_DISCONNECT = auto()
    SEND_DISCONNECT_ACK = auto()

    # EMERGENCY STATES
    ENTERING_EMERGENCY = auto()
    AWAITING_CONNECTION = auto()
    SEND_CONNECTION_ACK = auto()
    EMERGENCY_UPLINK = auto()

    SERVER_SIDE_ERROR = auto()
