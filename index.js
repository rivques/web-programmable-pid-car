try {
  let ws = new WebSocket("");
} catch {
  // error for empty string, just here for type hinting
}
let lastPacketTime = Date.now()
let chart = new Chart(document.getElementById("chart"), {
  type: "line",
  data: {
    labels: [],
    datasets: [
      {
        label: "Distance",
        data: [],
        yAxisID: "yOther",
        borderColor: "#0066ff",
        order: 2 
      },
      {
        label: "Absolute Error",
        data: [],
        yAxisID: "yOther",
        borderColor: "#ff3300",
        order: 1
      },
      {
        label: "Setpoint",
        data: [],
        yAxisID: "yOther",
        borderColor: "#9933ff",
        borderDash: [5, 10],
        order: 3 // draw on top
      },
      {
        label: "Output",
        data: [],
        yAxisID: "yOutput",
        borderColor: "#00bb00",
        order: 0
      },
    ],
  },
  options: {
    animation: {
      duration: 250 // speed up animation
    },
    elements: {
      point: {
        pointRadius: 0,
      },
    },
    scales: {
      xAxis: {
        // The axis for this scale is determined from the first letter of the id as `'x'`
        // It is recommended to specify `position` and / or `axis` explicitly.
        type: "linear",
        title: {
          display: true,
          text: "Car On Time (sec)",
        },
      },

      yOutput: {
        type: "linear",
        title: {
          display: true,
          text: "PID output",
        },
        display: true,
        position: "left",
        min: -1,
        max: 1,
        grid: {
          color: function(context) {
            if (context.tick.value == 0) {
              return "#00b300";
            }
            return 'rgba(0, 0, 0, 0.1)';
          },
        },
      },
      yOther: {
        type: "linear",
        title: {
          display: true,
          text: "Distance (cm)",
        },
        display: true,
        position: "right",
        min: 0,

        // grid line settings
        grid: {
          drawOnChartArea: false, // only want the grid lines for one axis to show up
        },
      },
    },
  },
});

function onWSConnect(e) {
  document.querySelector('#connect-form [type="submit"]').value = "Disconnect";
  document.querySelector('#connect-form [type="submit"]').disabled = false;
  document.getElementById("interface").style.display = "block";
}
function onWSMessage(e) {
  console.log("WebSocket message: " + e.data);
  lastPacketTime = Date.now()
  if (e.data.startsWith("G:")) {
    const gains = e.data.slice(2).split(",");
    document.getElementById("kP").value = gains[0];
    document.getElementById("kI").value = gains[1];
    document.getElementById("kD").value = gains[2];
    document.getElementById("setpoint").value = gains[3];
  } else if (e.data.startsWith("F:")) {
    const curState = e.data.slice(2);
    // document.getElementById("data").textContent = curState;
    if (curState.includes("F")) {
      return; // garbage data from first send
    }
    const parsedState = curState.split(","); // [distance, error, setpoint, output, time]
    chart.data.labels.push(parsedState[4]); // time
    maxVal = Math.max(...chart.data.labels.map((el) => parseFloat(el)));
    chart.options.scales.xAxis.max = maxVal;
    chart.options.scales.xAxis.min = maxVal - 20; // 20 second moving window
    for (let i = 0; i < chart.data.datasets.length; i++) {
      if(i != 1 ){
      chart.data.datasets[i].data.push(parsedState[i]);
      } else {
        // make the error an absolute value
        chart.data.datasets[i].data.push(Math.abs(parsedState[i]));
      }
    }
    chart.update();
    //chart.data.datasets[0].data.push({time, distance, error, setpoint, output})
  }
}
function onWSClose(e) {
  console.log("WebSocket closed, reason: " + e.reason);
  if (!e.wasClean) {
    alert("Error: WebSocket terminated, reason: " + e.reason);
  }
  document.getElementById("interface").style.display = "none";
  const children = document.querySelectorAll("#connect-form input");
  for (const child of children) {
    child.disabled = false;
  }
  document.querySelector('#connect-form [type="submit"]').value = "Connect";
}
document.getElementById("connect-form").onsubmit = (e) => {
  e.preventDefault(); // don't send a request to the server, everything is client-side
  // disable the form and try to connect
  if(document.querySelector('#connect-form [type="submit"]').value == "Disconnect"){
    ws.close()
    return;
  }
  const children = document.querySelectorAll("#connect-form input");
  for (const child of children) {
    child.disabled = true;
  }
  document.querySelector('#connect-form [type="submit"]').value =
    "Connecting...";
  try {
    ws = new WebSocket(
      `ws://${document.getElementById("ip-addr").value}:${document.getElementById("port").value
      }`
    );
  } catch {
    onWSClose(new CloseEvent("init error"));
    return false;
  }
  ws.onopen = onWSConnect;
  ws.onmessage = onWSMessage;
  ws.onclose = onWSClose;
  return false; // don't submit the form
};
function updateGains() {
  const stringToSend = `G:${document.getElementById("kP").value},${document.getElementById("kI").value
    },${document.getElementById("kD").value},${document.getElementById("setpoint").value
    }`;
  console.log(stringToSend);
  ws.send(stringToSend);
}
document.getElementById("kP").onchange = (e) => updateGains();
document.getElementById("kI").onchange = (e) => updateGains();
document.getElementById("kD").onchange = (e) => updateGains();
document.getElementById("setpoint").onchange = (e) => updateGains();
setInterval(() => {
  const timeOffset = Date.now() - lastPacketTime;
  document.getElementById("watchdog-timer").textContent = (timeOffset/1000).toFixed(2).toString()
}, 50)