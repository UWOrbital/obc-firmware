import type { RequestItemData } from "./request-item-data.ts";

export const getRequestItems = async (): Promise<RequestItemData[]> => {
  try {
    const response = await fetch('http://localhost:5000/aro-request');

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();

    return data.map((item: any) => ({
      ...item,
      status: item.status.toLowerCase(),
      created_on: new Date(item.created_on),
      request_sent_to_obc_on: item.request_sent_to_obc_on ? new Date(item.request_sent_to_obc_on) : null,
      pic_taken_on: item.pic_taken_on ? new Date(item.pic_taken_on) : null,
      pic_transmitted_on: item.pic_transmitted_on ? new Date(item.pic_transmitted_on) : null,
      cancellable_after: item.cancellable_after ? new Date(item.cancellable_after) : new Date(),
    }));
  } catch (error) {
    console.error('Error fetching ARO requests: ', error);
    throw error;
  }
};

