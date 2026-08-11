import { byId, communicationError, displayLcdText, simLcd } from "./shared";
import { BWF } from "./bwf";

function resize() {
    var width = document.documentElement.clientWidth - 20;
    var height = (width - 10) / 2.8;
    var fontsize = height / 4 - 10;
    var frame = document.getElementsByClassName("lcddisplay")[0];
    frame.style.width = width + "px";
    frame.style.height = height + "px";
    document.getElementsByClassName("lcd-text")[0].style.fontSize =
        parseInt(fontsize) + "px";
}

export function init() {
    resize();
    window.addEventListener(
        "resize",
        function () {
            resize();
        },
        false,
    );
    byId("lcd").onclick = function () {
        byId("myDropdown").classList.toggle("show");
        event.stopPropagation();
    };

    BWF.init({
        onconnect: function () {},
        error: function () {
            console.log("error");
            communicationError();
        },
        handlers: {
            A: function (info) {
                if (typeof info["sl"] != "undefined") {
                    displayLcdText(simLcd(info));
                }
            },
        },
    });
}

window.onclick = function () {
    const dropdowns = document.getElementsByClassName("dropdown-content");
    for (let i = 0; i < dropdowns.length; i++) {
        var openDropdown = dropdowns[i];
        if (openDropdown.classList.contains("show")) {
            openDropdown.classList.remove("show");
        }
    }
};
