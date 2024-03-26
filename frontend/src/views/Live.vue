<template>
  <div>
    <v-row>
      <v-col>
        <div id="container" style="width: 90%; max-width: 800px; margin: 0 auto;"></div>
      </v-col>

      <!-- col 2: -->
      <v-col cols="3">
        <!-- card 1: -->
        <v-card style="margin-bottom: 5px;" max-width="200px" color="primaryContainer" subtitle="Humidity">
            <v-card-item>
              <span class="text-onPrimaryContainer">{{ humidity }}</span>
            </v-card-item>
        </v-card>
        
        <!-- card 2: -->
        <v-card style="margin-bottom: 5px;" max-width="200px" color="tertiaryContainer" subtitle="Pressure">
            <v-card-item>
                <span class="text-onTertiaryContainer">{{ pressure }}</span>
            </v-card-item>
        </v-card>
        
        <!-- card 3: -->
        <v-card style="margin-bottom: 5px;" max-width="200px" color="quaternaryContainer" subtitle="Altitude">
            <v-card-item>
                <span class="text-onSecondaryContainer">{{ altitude }}</span>
            </v-card-item>
        </v-card>

        <!-- card 3: -->
        <v-card style="margin-bottom: 5px;" max-width="200px" color="quaternaryContainer" subtitle="Soil Moisture">
            <v-card-item>
                <span class="text-onSecondaryContainer">{{ soil_moisture }}</span>
            </v-card-item>
        </v-card>
      </v-col>
    </v-row>

    <div class="row">
      <div class="column" v-for="(data, label) in weatherData" :key="label">
        <div v-if="label !== 'temperature' && label !== 'heat_index' && label !== 'dew_point'">
          <div :id="label"></div>
          <!-- <div class="variable-name">{{ label }}: {{ data }}{{ unit[label] }}</div> -->
          <span class="text-onPrimaryContainer">{{ humidity }}</span>
          <span class="text-onPrimaryContainer">{{ pressure }}</span>
          <span class="text-onPrimaryContainer">{{ altitude }}</span>
          <span class="text-onPrimaryContainer">{{ soil_moisture }}</span>
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
  heat_index: 28,
  dew_point: 20,
  pressure: 1007,
  altitude: 51,
  soil_moisture: 40
};

const unit = {
  pressure: 'hPa',
  altitude: 'm',
  soil_moisture: '%',
  humidity: '%'
};

const percentageData = {};
const gauges = {}; // Store gauge instances

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


// LIFECYCLE HOOKS
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
    colGraphChart.value.series[1].addPoint({y:parseFloat(data.heat_index.toFixed(2)) ,x: data.timestamp * 1000 }, true, shift.value);
    colGraphChart.value.series[2].addPoint({y:parseFloat(data.dew_point.toFixed(2)) ,x: data.timestamp * 1000 }, true, shift.value);
});

watch(payload, (data) => {
  updateRingMeters();
});

// create column graphs:
const CreateColumnGraph = async () => {
    colGraphChart.value = Highcharts.chart('container', {
        chart: { zoomType: 'x' },
        title: { text: 'Air Temperature, Heat Index, and Dew Point Analysis', align: 'left' },
        yAxis: {
            title: { text: 'Value °C', style: { color: '#000000' } },
            labels: { format: '{value} °C' }
        },
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
            },
            {
                name: 'Dew Point',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: '#31EE16', // Change color for dew point column
                // dataLabels: { enabled: true, color: 'black', formatter: function() { return this.y + '°C'; } } // Display value above column
            }
        ],
    });
};

// create ring meters:
const CreateRingMeters = () => {
  for (const [label, raw_value] of Object.entries(payload.value)) {
    console.log(payload.value);
    if (label === 'humidity' || label === 'pressure' || label === 'altitude' || label === 'soil_moisture') {
      createRingMeter(label, raw_value);
    }
  }
};

const createRingMeter = (label, raw_value) => {
  console.log("Label:", label, "Value:", raw_value); // Debug statement
  const range = getRange(label);
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


// update ring meters:
const updateRingMeters = () => {
  for (const [label, raw_value] of Object.entries(payload.value)) {
    if (label === 'humidity' || label === 'pressure' || label === 'altitude' || label === 'soil_moisture') {
      updateRingMeter(label, raw_value);
    }
  }
};

const updateRingMeter = (label, raw_value) => {
  const range = getRange(label);
  const percentage = ((raw_value - range.min) / (range.max - range.min)) * 100;
  percentageData[label] = raw_value;
  if (gauges[label]) {
    gauges[label].refresh(raw_value);
  }
};

const getRange = (label) => {
  if (label === 'humidity') {
    return { min: 0, max: 100 };
  } else if (label === 'pressure') {
    return { min: 888, max: 1031 };
  } else if (label === 'altitude') {
    return { min: 0, max: 2256 };
  } else if (label === 'soil_moisture') {
    return { min: 0, max: 100 };
  }
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
    margin: 10px;
}
.variable-name {
    margin-top: 5px;
    margin-left: 90px;
}
</style>