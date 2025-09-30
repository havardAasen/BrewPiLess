import { init as graph } from "./script";
import { initctrl as control } from "./script-control";
import { init as logging } from "./script-logging";
import { init as setup } from "./script-setup";
import { load as config } from "./script-config";
import { init as gravity } from "./script-gravity";
import { loaded as pressure } from "./script-pressure";
import { init as lcd } from "./script-lcd";
import { loaded as logViewer } from "./script-viewer";
import { loaded as logWebViewer } from "./script-webviewer"

document.addEventListener('DOMContentLoaded', () => {
  const pageId = document.body.dataset.page;

  switch (pageId) {
    case 'graph':
      graph()
      break;
    case 'control':
      control()
      break;
    case 'logging':
      logging()
      break;
    case 'setup':
      setup()
      break;
    case 'config':
      config()
      break;
    case 'gravity':
      gravity()
      break;
    case 'pressure':
      pressure()
      break;
    case 'lcd':
      lcd()
      break;
    case 'logViewer':
      logViewer()
      break;
    case 'logWebViewer':
      logWebViewer()
      break;
    default:
      console.warn('No page initializer found');
  }
});
