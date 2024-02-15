<template>
    <v-container fluid background-color="surface" align="center">
        <!-- row 1: -->
        <v-row style="max-width: 1200px;">
            <!-- col 1: -->
            <v-col cols="9" >
                <figure class="highcharts-figure">
                    <div id="container">
                        <!-- Temperature graph -->
                    </div>
                </figure>
            </v-col>

            <!-- col 2: -->
            <v-col cols="3">
                <!-- card 1: -->
                <v-card style="margin-bottom: 5px;" max-width="500px" color="primaryContainer" subtitle="Temperature">
                    <v-card-item>
                        <span class="text-h3 text-onPrimaryContainer">{{ temperature }}</span>
                    </v-card-item>
                </v-card>
                
                <!-- card 2: -->
                <v-card style="margin-bottom: 5px;" max-width="500px" color="tertiaryContainer" subtitle="Heat Index (Feels like)">
                    <v-card-item>
                        <span class="text-h3 text-onTertiaryContainer">{{ heatindex }}</span>
                    </v-card-item>
                </v-card>
                
                <!-- card 3: -->
                <v-card style="margin-bottom: 5px;" max-width="500px" color="quaternaryContainer" subtitle="Humidity">
                    <v-card-item>
                        <span class="text-h3 text-onSecondaryContainer">{{ humidity }}</span>
                    </v-card-item>
                </v-card>
            </v-col>
        </v-row>

        <!-- row 2: -->
        <v-row style="max-width: 1200px;" justify="start">
            <v-col cols="9">
                <figure class="highcharts-figure">
                    <div id="container1">
                        <!-- Humidity graph -->
                    </div>
                </figure>
            </v-col>
        </v-row>
</v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";
import { useMqttStore } from '@/store/mqttStore'; // Import Mqtt Store
import { storeToRefs } from "pinia";

// Highcharts, Load the exporting module and Initialize exporting module.
import Highcharts from 'highcharts';
import more from 'highcharts/highcharts-more';
import Exporting from 'highcharts/modules/exporting';
Exporting(Highcharts);
more(Highcharts);
 
// VARIABLES
const router                    = useRouter();
const route                     = useRoute();
const Mqtt                      = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);
const tempHiChart               = ref(null);        // Temp Chart object
const humHiChart                = ref(null);        // Hum Chart object
const points                    = ref(10);          // Specify the quantity of points to be shown on the live graph simultaneously.
const shift                     = ref(false);       // Delete a point from the left side and append a new point to the right side of the graph.

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

// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();
    // CreateHumidChart();

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

 tempHiChart.value.series[0].addPoint({y:parseFloat(data.temperature.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);

 tempHiChart.value.series[1].addPoint({y:parseFloat(data.heatindex.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);

humHiChart.value.series[0].addPoint({y:parseFloat(data.humidity.toFixed(2)) ,x: data.timestamp * 1000 },
true, shift.value);
})

const CreateCharts = async () => {
    // TEMPERATURE & HEAT INDEX CHART:
    tempHiChart.value = Highcharts.chart('container', {
        chart: { zoomType: 'x' },
        title: { text: 'Air Temperature and Heat Index Analysis', align: 'left' },
        yAxis: {
            title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}},
            labels: { format: '{value} °C' }
        },
        xAxis: {
            type: 'datetime',
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { shared:true, },
        series: [
            {
                name: 'Temperature',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[0]
            },
        {
            name: 'Heat Index',
            type: 'spline',
            data: [],
            turboThreshold: 0,
            color: Highcharts.getOptions().colors[1]
        }],
    });

    // HUMIDITY CHART:
    humHiChart.value = Highcharts.chart('container1', {
        chart: { zoomType: 'x' },
        title: { text: 'Humidity Analysis', align: 'left' },
        yAxis: {
            title: { text: 'Humidity' , style:{color:'#000000'}},
            labels: { format: '{value} %' }
        },
        xAxis: {
            type: 'datetime',
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { shared:true, },
        series: [
            {
                name: 'Humidity',
                type: 'spline',
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[2]
            }],
    });
};
</script>

<style scoped>
/** CSS STYLE HERE */
Figure {
    border: 2px solid black;
}
</style>