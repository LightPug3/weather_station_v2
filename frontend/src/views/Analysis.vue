<template>
    <v-container background-color="surface" fluid>
        <!-- row 1: -->
        <v-row style="max-width: 1200px; padding: 1px;">
            <!-- col 1: -->
            <v-col>
                <v-sheet padding="2" height="250">
                    <p>Enter data range for Analysis</p>
                    <v-divider></v-divider>
                    <v-text-field label="Start date" type="Date" density="compact" variant="solo-inverted" style="margin-right: 5px; max-width: 330px;" flat v-model="start"></v-text-field>
                    <v-spacer></v-spacer>
                    <v-text-field label="End date" type="Date" density="compact" variant="solo-inverted" style="margin-right: 5px; max-width: 330px;" flat v-model="end"></v-text-field>
                    <v-spacer></v-spacer>
                    <VBtn @click="updateLineCharts(); updateCards(); updateHistogramCharts(); updateScatter();" text="Analyze" color="primary" variant="tonal"></VBtn>
                </v-sheet>
            </v-col>

            <!-- col 2: -->
            <v-col cols="3" align="center">
                <v-card title="Temperature" width="250" variant="outlined" color="primary" density="compact" rounded="lg">
                    <v-card-item margin-bottom="-5">
                        <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
                            <v-tooltip text="Min" location="start">
                                <!-- <v-chip>{{ temperature.min }}</v-chip> -->
                            </v-tooltip>

                            <v-tooltip text="Range" location="top">
                                <!-- <v-chip>{{ temperature.range }}</v-chip> -->
                            </v-tooltip>

                            <v-tooltip text="Max" location="end">
                                <!-- <v-chip>{{ temperature.max }}</v-chip> -->
                            </v-tooltip>
                        </v-chip-group>
                    </v-card-item>

                    <v-card-item align="center">
                        <!-- <span class="text-h1 text-primary font-weight-bold">{{ temperature.avg }}</span> -->
                    </v-card-item>
                </v-card>
            </v-col>

            <!-- col 3: -->
            <v-col cols="3" align="center">
                <v-card title="Humidity" width="250" variant="outlined" color="primary" density="compact" rounded="lg">
                    <v-card-item margin-bottom="-5">
                        <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
                            <v-tooltip text="Min" location="start">
                                <!-- <v-chip>{{ humidity.min }}</v-chip> -->
                            </v-tooltip>

                            <v-tooltip text="Range" location="top">
                                <!-- <v-chip>{{ humidity.range }}</v-chip> -->
                            </v-tooltip>

                            <v-tooltip text="Max" location="end">
                                <!-- <v-chip>{{ humidity.max }}</v-chip> -->
                            </v-tooltip>
                        </v-chip-group>
                    </v-card-item>

                    <v-card-item align="center">
                        <!-- <span class="text-h1 text-primary font-weight-bold">{{ humidity.avg }}</span> -->
                    </v-card-item>
                </v-card>
            </v-col>
        </v-row>

        <!-- row 2: -->
        <v-row style="max-width: 1200px;">
            <!-- col 1: -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div class="container"><!--  --></div>
                </figure>
            </v-col>

            <!-- col 2: -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div class="container0"><!--  --></div>
                </figure>
            </v-col>
        </v-row>
        
        <!-- row 3: -->
        <v-row style="max-width: 1200px;">
            <!-- col 1: -->
            <v-col cols="12" border>
                <figure class="highcharts-figure">
                    <div class="container1"><!--  --></div>
                </figure>
            </v-col>

            <!-- col 2: -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div class="container2"><!--  --></div>
                </figure>
            </v-col>

            <!-- col 3: -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div class="container3"><!--  --></div>
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
 
 
// VARIABLES
const router                        = useRouter();
const route                         = useRoute();
const start                         = ref("");
const end                           = ref("");
const tempHiChart                   = ref(null);        // Temp & HI Chart object
const humHiChart                    = ref(null);        // Humidity Chart object
const colGraphChart                 = ref(null);        // Temp, Humidity & HI Chart Object
const tempHIScatPlot                = ref(null);        // Temp & HI Scatter Plot Object
const humScatPlot                   = ref(null);        // Humidity Scatter Plot Object

// Create two (2) reactive variables in the <script> section of Analysis.vue called ‘temperature’ and ‘humidity’.
const temperature = reactive({"min":0,"max":0,"avg":0,"range":0});
const humidity    = reactive({"min":0,"max":0,"avg":0,"range":0});


// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();

    Mqtt.connect(); // Connect to Broker located on the backend

    setTimeout(()=>{
        // Subscribe to each topic
        Mqtt.subscribe("620156694");
        Mqtt.subscribe("620156694_sub");
    },3000);
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});

const updateLineCharts = async () => {
    if (!!start.value && !!end.value) {
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Fetch data from backend
        const data = await AppStore.getAllInRange(startDate, endDate);
        // Create arrays for each plot
        let temperature = [];
        let heatindex = [];
        let humidity = [];
        // Iterate through data variable and transform object to format recognized by highcharts
        data.forEach((row) => {
            temperature.push({
                x: row.timestamp * 1000,
                y: parseFloat(row.temperature.toFixed(2)),
            });
            heatindex.push({
                x: row.timestamp * 1000,
                y: parseFloat(row.heatindex.toFixed(2)),
            });
            humidity.push({
                x: row.timestamp * 1000,
                y: parseFloat(row.humidity.toFixed(2)),
            });
        });
        // Add data to Temperature and Heat Index chart
        tempHiLine.value.series[0].setData(temperature);
        tempHiLine.value.series[1].setData(heatindex);
        humLine.value.series[0].setData(humidity);
    }
};

const updateCards = async () => {
    // Retrieve Min, Max, Avg, Spread/Range
    if (!!start.value && !!end.value) {
        // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // 2. Fetch data from backend by calling the API functions
        const temp = await AppStore.getTemperatureMMAR(startDate, endDate);
        const humid = await AppStore.getHumidityMMAR(startDate, endDate);
        //console.log(temp);
        temperature.max = temp[0].max.toFixed(1);
        temperature.min = temp[0].min.toFixed(1);
        temperature.avg = temp[0].avg.toFixed(1);
        temperature.range = temp[0].range.toFixed(1);
        humidity.max = humid[0].max.toFixed(1);
        humidity.min = humid[0].min.toFixed(1);
        humidity.avg = humid[0].avg.toFixed(1);
        humidity.range = humid[0].range.toFixed(1);
    }
};

const updateHistogramCharts = async () => {
    // Retrieve Min, Max, Avg, Spread/Range for Column graph
    if (!!start.value && !!end.value) {
        // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps:
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;

        // 2. Fetch data(temp, humid and hi) from backend by calling the getFreqDistro API functions for each:
        const temp = await AppStore.getFreqDistro("temperature", startDate, endDate);
        const humid = await AppStore.getFreqDistro("humidity", startDate, endDate);
        const hi = await AppStore.getFreqDistro("heatindex", startDate, endDate);
        
        // 3. create an empty array for each variable (temperature, humidity and heatindex): see example below
        let temperature = [];
        let humidity = [];
        let heatindex = [];

        // 4. Iterate through the temp variable, which contains temperature data fetched from the backend
        // transform the data to {"x": x_value,"y": y_value} format and then push it to the temperature array created previously. see example below:
        temp.forEach((row) => {
            temperature.push({ x: row["_id"], y: row["count"] });
        });
        humid.forEach((row) => {
            humidity.push({ x: row["_id"], y: row["count"] });
        });
        hi.forEach((row) => {
            heatindex.push({ x: row["_id"], y: row["count"] });
        });
        colGraphChart.value.series[0].setData(temperature);
        colGraphChart.value.series[1].setData(humidity);
        colGraphChart.value.series[2].setData(heatindex);
    }
};

// CHARTS:
const CreateCharts = async () => {
    // TEMPERATURE & HEAT INDEX CHART:
    tempHiChart.value = Highcharts.chart('container', {
        chart: { zoomType: 'x' },
        title: { text: 'Temperature and Heat Index Analysis', align: 'left' },
        subtitle:{ text:'The heat index, also known as the "apparent temperature," is a measure that combines air temperature and relative humidity to assess how hot it feels to the human body. The relationship between heat index and air temperature is influenced by humidity levels. As humidity increases, the heat index also rises, making the perceived temperature higher than the actual air temperature.' },
        yAxis: {
            title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'} },
            labels: { format: '{value} °C' }
        },
        xAxis: {
            type: 'datetime',
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { shared:true, pointFormat: 'Humidity: {point.x} % <br/> Temperature: {point.y} °C'},
        series: [
            {
                name: 'Temperature',
                type: 'line',
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[0]
            },
        {
            name: 'Heat Index',
            type: 'line',
            data: [],
            turboThreshold: 0,
            color: Highcharts.getOptions().colors[1]
        }],
    });

    // HUMIDITY CHART:
    humHiChart.value=Highcharts.chart('container0', {
        chart: { zoomType: 'x' },
        title: { text: 'Humidity Analysis', align: 'left' },
        yAxis: { 
            title: { text: 'Humidity' , style:{color:'#000000'} },
            labels: { format: '{value} %' } 
        },
        xAxis: { 
            type: 'datetime', 
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { 
            shared: true, pointFormat: 'Humidity: {point.x} % <br/> Temperature: {point.y} °C'},
        series: [
            {
                name: "Humidity",
                type: "line",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[2],
            },
        ]
    });

    // Temp, Humidity & HI COLUMN GRAPH:
    colGraphChart.value = Highcharts.chart("container1", {
        chart: { zoomType: "x" },
        title: { text: "Frequency Distribution Analysis", align: "left" },
        yAxis: { 
            title: { text: "Frequency", style: { color: "#000000" } },
            labels: { format: "{value}" },
            },

        xAxis: {
            title: { text: "ID", style: { color: "#000000" } },
        },
        tooltip: { shared: true },
        series: [
            {
                name: "Temperature",
                type: "bar",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[3],
            },
            {
                name: "Humidity",
                type: "bar",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[4],
            },
            {
                name: "Heat Index",
                type: "bar",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[5],
            },
        ],
    });

    // Temp & HI Scatter PLOT:
    tempHIScatPlot.value = Highcharts.chart("container2", {
        chart: { zoomType: "x" },
        title: { text: "Temperature & Heat Index Correlation Analysis", align: "left",},
        subtitle: { text: "Visualize the relationship between Temperature and Heat Index as well as revealing patterns or trends in the data",},
        
        yAxis: {
            title: { text: "Heat Index", style: { color: "#000000" } },
            labels: { format: "{value} °C" },
        },
        xAxis: {
            title: { text: "Temperature", style: { color: "#000000" } },
            labels: { format: "{value} °C" },
        },
        tooltip: { shared: true, pointFormat: "Temperature: {point.x} °C <br/> Heat Index: {point.y} °C"},
        series: [
            {
                name: "Analysis",
                type: "scatter",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[6],
            },
        ],
    });

    // Humidity Scatter PLOT:
    humScatPlot.value = Highcharts.chart("container3", {
        chart: { zoomType: "x" },
        title: { text: "Humidity & Heat Index Correlation Analysis", align: "left" },
        subtitle: { text: "Visualize the relationship between Humidity and Heat Index as well as revealing patterns or trends in the data" },
        yAxis: {
            title: {
                text: "Heat Index",
                style: { color: "#000000" },
            },
            labels: { format: "{value} °C" },
        },

        xAxis: {
            title: { text: "Humidity", style: { color: "#000000" } },
            labels: { format: "{value} %" },
        },
        tooltip: { shared: true, pointFormat: "Humidity: {point.x} °C <br/> Heat Index: {point.y} °C" },
        series: [
            {
                name: "Analysis",
                type: "scatter",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[7],
            },
        ],
    });
};
</script>


<style scoped>
/** CSS STYLE HERE */


</style>
  