<template>
  <div>
    <div id="container" style="width: 60%; max-width: 800px; margin: 0 auto;"></div>
    <div class="row">
      <div class="column" v-for="(data, label) in weatherData" :key="label">
        <div v-if="label !== 'temperature' && label !== 'heatIndex' && label !== 'dewPoint'">
          <div :id="label"></div>
          <div class="variable-name">{{ label }}: {{ data }}{{ unit[label] }}</div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
/** JAVASCRIPT HERE */
// IMPORTS
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";
import { useMqttStore } from '@/store/mqttStore'; // Import Mqtt Store
import { storeToRefs } from "pinia";

import Highcharts from 'highcharts';
import JustGage from 'justgage';
import more from 'highcharts/highcharts-more';
import Exporting from 'highcharts/modules/exporting';
Exporting(Highcharts);
more(Highcharts);

// VARIABLES
const router                    = useRouter();
const route                     = useRoute();
const Mqtt                      = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);
const colGraphChart             = ref(null);
const points                    = ref(10);          // Specify the quantity of points to be shown on the live graph simultaneously.
const shift                     = ref(false);       // Delete a point from the left side and append a new point to the right side of the graph.

const weatherData = {
  temperature: 25,
  humidity: 60,
  heatIndex: 28,
  dewPoint: 20,
  pressure: 1007,
  altitude: 51,
  soilMoisture: 40
};

const unit = {
  pressure: 'hPa',
  altitude: 'm',
  soilMoisture: '%',
  humidity: '%'
};

const percentageData = {};


// COMPUTED PROPERTIES
const temperature = computed(()=>{
    if(!!payload.value){
        return `${payload.value.temperature.toFixed(2)} °C`;
    }
});

const heatindex = computed(()=>{
    if(!!payload.value){
        return `${payload.value.heatindex.toFixed(2)} °C`;
    }
});

const humidity = computed(()=>{
    if(!!payload.value){
        return `${payload.value.humidity.toFixed(2)} %`;
    }
});

const pressure = computed(()=>{
    if(!!payload.value){
        return `${payload.value.pressure.toFixed(2)} hPa`;
    }
});

const altitude = computed(()=>{
    if(!!payload.value){
        return `${payload.value.altitude.toFixed(2)} m`;
    }
});

const soil_moisture = computed(()=>{
    if(!!payload.value){
        return `${payload.value.soil_moisture.toFixed(2)} %`;
    }
});



onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateColumnGraph();
    CreateRingMeters();

    Mqtt.connect(); // Connect to Broker located on the backend

    setTimeout( ()=>{
        // Subscribe to each topic
        Mqtt.subscribe("620156694");
        Mqtt.subscribe("620156694_pub");
    },3000);
});

onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});


// WATCHERS
watch(payload,(data)=> {
    if(points.value > 0){ points.value -- ; }
    else{ shift.value = true; }

    colGraphChart.value.series[0].addPoint({y:parseFloat(data.temperature.toFixed(2)) ,x: data.timestamp * 1000 }, true, shift.value);

    colGraphChart.value.series[1].addPoint({y:parseFloat(data.heatindex.toFixed(2)) ,x: data.timestamp * 1000 }, true, shift.value);

    // colGraphChart.value.series[2].addPoint({y:parseFloat(data.humidity.toFixed(2)) ,x: data.timestamp * 1000 }, true, shift.value);
})

// create column graphs:
const CreateColumnGraph = async () => {
    colGraphChart.value = Highcharts.chart('container', {
        // chart: { type: 'column' },                      // Change type to 'column' for column graph
        chart: { zoomType: 'x' },
        title: { text: 'Air Temperature, Heat Index, and Dew Point Analysis', align: 'left' },
        yAxis: {
            title: { text: 'Value °C', style: { color: '#000000' } },
            labels: { format: '{value} °C' }
        },
        // xAxis: {
        //     title: { text: 'Variables', style: { color: '#000000' } },
        // },
        xAxis: {
            type: 'datetime',
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { shared: true },
        series: [
            {
                name: 'Temperature',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: '#EA2525', // Change color for temperature column
                // dataLabels: { enabled: true, color: 'black', formatter: function() { return this.y + '°C'; } } // Display value above column
            },
            {
                name: 'Heat Index',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: '#FF7400', // Change color for heat index column
                // dataLabels: { enabled: true, color: 'black', formatter: function() { return this.y + '°C'; } } // Display value above column
            }/*,
            {
                name: 'Dew Point',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: '#31EE16', // Change color for dew point column
                // dataLabels: { enabled: true, color: 'black', formatter: function() { return this.y + '°C'; } } // Display value above column
            }*/
        ],
    //     chart: {
    //         type: 'column',
    //     },
    //         title: {
    //         text: 'Temperature Related Data'
    //     },
    //         xAxis: {
    //         categories: ['Temperature', 'Heat Index', 'Dew Point']
    //     },
    //         yAxis: {
    //         title: {
    //             text: 'Unit: °C'
    //         }
    //     },
    //     series: [{
    //         data: [
    //             {
    //                 y: /*weatherData.*/temperature,
    //                 dataLabels: {
    //                     enabled: true,
    //                     format: /*weatherData.*/temperature + '°C',
    //                     color: 'black'
    //                 }
    //             },
    //             {
    //                 y: /*weatherData.*/heatindex,
    //                 dataLabels: {
    //                     enabled: true,
    //                     format: /*weatherData.*/heatindex + '°C',
    //                     color: 'black'
    //                 }
    //             },
    //             {
    //                 y: /*weatherData.*/humidity,
    //                 dataLabels: {
    //                     enabled: true,
    //                     format: /*weatherData.*/humidity + '°C',
    //                     color: 'black'
    //                 }
    //             }
    //         ]
    //     }],
    //     plotOptions: {
    //         column: {
    //             colorByPoint: true,
    //             colors: ['#EA2525', '#FF7400', '#31EE16']
    //         }
    //     }
    });
};

// create ring meters:
const CreateRingMeters = () => {
  for (const [label, raw_value] of Object.entries(weatherData)) {
    if (label === 'pressure' || label === 'altitude' || label === 'soilMoisture' || label === 'humidity') {
      createRingMeter(label, raw_value);
    }
  }
};

const createRingMeter = (label, raw_value) => {
  let range;
  if (label === 'pressure') {
    range = { min: 888, max: 1031 };
  } else if (label === 'altitude') {
    range = { min: 0, max: 2256 };
  } else if (label === 'soilMoisture') {
    range = { min: 0, max: 100 };
  } else if (label === 'humidity') {
    range = { min: 0, max: 100 };
  }
  const percentage = ((raw_value - range.min) / (range.max - range.min)) * 100;
  percentageData[label] = raw_value;
  const div = document.createElement('div');
  div.id = label;
  document.querySelector('.row').appendChild(div);
  const gauge = new JustGage({
    id: label,
    value: raw_value,
    min: range.min,
    max: range.max,
    title: label.charAt(0).toUpperCase() + label.slice(1),
    label: unit[label],
    donut: true,
    gaugeWidthScale: 0.4,
    counter: true,
    relativeGaugeSize: true,
    levelColors: ['#F39C12', '#01276', '#09A4E7', '#E74C3C']
  });
  gauge.refresh(raw_value);
};
</script>

<style scoped>
/** CSS STYLE HERE */
.row {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    align-items: center;
    margin-top: 20px;
}
.column {
    /*width: 200px; /* Adjust the width as needed */
    margin: 10px;
    /* text-align: center; */
}
.variable-name {
    margin-top: 5px;
    /* font-size: 14px; */
    margin-left: 90px;
}
</style>