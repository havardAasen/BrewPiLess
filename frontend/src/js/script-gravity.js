import { select, getActiveNavItem, JSVERSION, s_ajax } from './shared';

var gdcurl = "/gdc";

function fill(setting) {
    for (var name in setting) {
        var ele = select("input[name=" + name + "]");
        if (ele) {
            if (ele.type == "checkbox") ele.checked = setting[name];
            else ele.value = setting[name];
        }
    }
}

function save() {
    var inputs = document.getElementsByTagName("input");
    var setting = {};
    for (var i = 0; i < inputs.length; i++) {
        var ele = inputs[i];
        if (ele.type == "checkbox") setting[ele.name] = ele.checked;
        else if (ele.type == "text") setting[ele.name] = ele.value;
    }
    //    console.log("result=" + setting);
    s_ajax({
        url: gdcurl,
        m: "POST",
        mime: "application/json",
        data: JSON.stringify(setting),
        success: function(a) {
            alert("<%= done %>");
        },
        fail: function(a) {
            alert("<%= script_control_failed_updating_data %>" + a)
        }
    });
}

export function init() {
    getActiveNavItem();
    select("#verinfo").innerHTML = "v" + JSVERSION;

    s_ajax({
        url: gdcurl + "?data",
        m: "GET",
        success: function(a) {
            fill(JSON.parse(a));
        },
        fail: function(a) {
            //alert("failed getting data:" + a)
        }
    });
}

window.save = save;
