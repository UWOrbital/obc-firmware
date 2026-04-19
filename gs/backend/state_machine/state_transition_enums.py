from enum import StrEnum, auto


class StateTransition(StrEnum):
    """
    The transition states of the ground station state machine
    """

    # TODO rename / clarify the specific transition states
    ERROR = auto()

    # UPLINK TRANSITION STATES
    BEGIN_UPLINK = auto()
    CONNECTION_ESTABLISHED = auto()
    ACK_RECEIVED = auto()

    # UPLINK / DOWNLINK TRANSITION STATES
    UPLINK_FINISHED = auto()
    DOWNLINKING_FINISHED = auto()

    # DISCONNECT TRANSITION STATES
    DISCONNECTING = auto()
    DISCONNECT_CMD_RECEIVED = auto()
    DISCONNECT_COMPLETE = auto()

    # EMERGENCY TRANSITION STATES
    ENTER_EMERGENCY = auto()
    EMERGENCY_INITIATED = auto()
    CONNECTION_RECEIVED = auto()
    CONNECTION_ACK_SENT = auto()
    EMERGENCY_UPLINK_FINISHED = auto()

    NO_TRANSITION_TRIGGERED = auto()  # NOT USED IN STATE MACHINE
