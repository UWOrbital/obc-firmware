/**
 * Routes for the ARO frontend
 */

export interface Route {
    text: string;
    url: string;
  }

  export const NAVIGATION_LINKS: Route[] = [
    {
      text: "Home",
      url: "/",
    },
    {
      text: "New",
      url: "/new-request",
    },
    {
      text: "Requests",
      url: "/requests",
    },
  ];
