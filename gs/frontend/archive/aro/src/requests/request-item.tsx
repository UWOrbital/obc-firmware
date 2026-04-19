import type { MouseEvent } from "react";
import type { RequestItemData } from "./request-item-data.ts";

export interface RequestItemProp extends RequestItemData {
  cancelRequest: (_: MouseEvent) => Promise<void>;
}

const RequestItem = (props: RequestItemProp) => {
  // TODO: Implement this
  const downloadPacket = async (a: MouseEvent) => {
    a.preventDefault();
    console.log(`Downloading packet for ${props.id}`);
  };

  return (
    <>
      <td>{props.id}</td>
      <td>{props.latitude}</td>
      <td>{props.longitude}</td>
      <td>{props.status}</td>
      <td>{props.created_on.toString()}</td>
      <td>{props.request_sent_to_obc_on?.toString()}</td>
      <td>{props.pic_taken_on?.toString()}</td>
      <td>{props.pic_transmitted_on?.toString()}</td>
      <td>
        <button onClick={downloadPacket}>Download Packet {props.id}</button>
      </td>
      <td>
        <button
          disabled={new Date() >= props.cancellable_after}
          onClick={props.cancelRequest}
        >
          Cancel Request {props.id}
        </button>
      </td>
    </>
  );
};

export default RequestItem;
