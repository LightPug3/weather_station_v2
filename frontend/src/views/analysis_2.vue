<template>
    <v-container fluid align=" center" color=" surface"  >
      <v-row class="mx-2" max-width="1200px" justify="center" align="center" padding="1">
          <v-col class="col col1" >
              <v-sheet class="mx-2" height="250">
                  <p>Enter date range for Analysis</p>
                  <v-divider></v-divider>
                  <v-text-field label="Start date" type="Date" density="compact" variant="solo-inverted" class="mr-5" max-width="300px" flat v-model="start"></v-text-field>
                  <v-text-field label="End date" type="Date" density="compact" variant="solo-inverted" class="mr-5" max-width="300px" flat v-model="end"></v-text-field>
                  
                  <v-spacer></v-spacer>
                  <VBtn @click="updateLineCharts(); updateCards(); updateHistogramCharts();" text="Analyze" color="primary" variant="tonal"></VBtn>
                  </v-sheet> 
          </v-col>

          <v-col class="col col2" cols="3" align="center"> 
              <v-card title="Temperature" width="250" variant="outlines" color="primary" density="compact" rounded="lg" border>
                  <v-card-item class="mb-n5">
                      <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
                          <v-tooltip text="Min" location="start">
                              <template v-slot:activator="{props}">
                                  <v-chip v-bind="props">{{ temperature.min }}</v-chip>
                              </template>             
                          </v-tooltip>
                          <v-tooltip text="Range" location="top">
                              <template v-slot:activator="{props}">
                                  <v-chip v-bind="props">{{ temperature.range }}</v-chip>
                              </template>     
                          </v-tooltip>
                          <v-tooltip text="Max" location="end">
                              <template v-slot:activator="{props}">
                                  <v-chip v-bind="props">{{ temperature.max }}</v-chip>
                              </template>     
                          </v-tooltip>
                      </v-chip-group>
                  </v-card-item>
                  <v-card-item align="center">
                      <span class="text-h1 text-primary font-weight-bold">
                          {{ temperature.avg }}
                      </span>
                  </v-card-item>
              </v-card>
          </v-col>
<!-- row1 col 3 EDIT THE TOOLTIPS-->
      <v-col class="col col3" cols="3" align="center"> 
                      <v-card title="Humidity" width="250" variant="outlines" color="primary" density="compact" rounded="lg" border>
                          <v-card-item class="mb-n5">
                              <v-chip-group class="d-flex flex-row justify-center" color="primaryContainer" variant="flat">
                                  <v-tooltip text="Min" location="start">
                                      <template v-slot:activator="{props}">
                                          <v-chip v-bind="props">{{ humidity.min }}</v-chip>
                                      </template>             
                                  </v-tooltip>
                                  <v-tooltip text="Range" location="top">
                                      <template v-slot:activator="{props}">
                                          <v-chip v-bind="props">{{ humidity.range }}</v-chip>
                                      </template>     
                                  </v-tooltip>
                                  <v-tooltip text="Max" location="end">
                                      <template v-slot:activator="{props}">
                                          <v-chip v-bind="props">{{ humidity.max }}</v-chip>
                                      </template>     
                                  </v-tooltip>
                              </v-chip-group>
                          </v-card-item>
                          <v-card-item align="center">
                              <span class="text-h1 text-primary font-weight-bold">
                                  {{ humidity.avg }}
                              </span>
                          </v-card-item>
                      </v-card>
                  </v-col>
      </v-row>

      <v-row max-width="1200px" justify="start" align="center">
          <v-col class="col col1" cols="12" align = 'center'>
              <figure class="highcharts-figure">
                  <div id="container"></div>
              </figure>
          </v-col>

          <v-col class="col col2" cols="12" align="center">
              <figure class="highcharts-figure">
                  <div id="container0"></div>
              </figure>
          </v-col>
      </v-row>

      <v-row max-width="1200px" justify="start" align="center">
          <v-col class="col col1" cols="12" align="center" style="border-right: 2px solid black;">
              <figure class="highcharts-figure">
                  <div id="container1"></div>
              </figure>
          </v-col>

          <v-col class="col col2" cols="12" align="center">
              <figure class="highcharts-figure">
                  <div id="container2"></div>
              </figure>
          </v-col>

          <v-col class="col col3" cols="12" align="center">
              <figure class="highcharts-figure">
                  <div id="container3"></div>
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
import { useAppStore } from "@/store/appStore";
import Highcharts from 'highcharts';
import more from 'highcharts/highcharts-more';
import { useMqttStore } from '@/store/mqttStore'; // Import Mqtt Store
import { storeToRefs } from "pinia";
import Exporting from 'highcharts/modules/exporting';
Exporting(Highcharts);
more(Highcharts);

// VARIABLES
const router      = useRouter();
const route       = useRoute();  
const Mqtt = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);
const AppStore = useAppStore();
const start = ref("");
const end = ref("");
const tempHiChart = ref(null); // Chart object
const humHiChart = ref(null); // Chart object
const colHiChart = ref(null); // Chart object
const scatHiChart = ref(null); // Chart object
const scat2HiChart = ref(null); // Chart object

// FUNCTIONS
// Define reactive variables for temperature and humidity
const temperature = reactive({"min":0,"max":0,"avg":0,"range":0});

const humidity = reactive({"min":0,"max":0,"avg":0,"range":0});

// FUNCTIONS
const CreateCharts = async () => {
  // TEMPERATURE CHART
  tempHiChart.value = Highcharts.chart('container', {
  chart: { zoomType: 'x' },
  title: { text: 'Temperature and Heat Index Analysis', align: 'left' },
  subtitle:{text:'The heat index, also known as the "apparent temperature," is a measure that combines air temperature and relative humidity to assess how hot it feels to the human body. The relationship between heat index and air temperature is influenced by humidity levels. As humidity increases, the heat index also rises, making the perceived temperature higher than the actual air temperature.', },
  yAxis: { 
      title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}},
      labels: { format: '{value} °C' } 
  },
  xAxis: {
      type: 'datetime', 
      title: { text: 'Time', style:{color:'#000000'} },
  },
  tooltip: { shared:true, pointFormat: 'Humidity: {point.x} % <br/> Temperature: {point.y} °C' 
      },
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
  humHiChart.value=Highcharts.chart('container0', {
      chart: { zoomType: 'x' },
      title: { text: 'Humidity Analysis', align: 'left' },
      yAxis: { 
          title: { text: 'Air Temperature & Heat Index' },
          labels: { format: '{value} %' } 
      },
      xAxis: {
          type: 'datetime', 
          title: { text: 'Time' },
      },
      tooltip: { 
          shared: true,
          pointFormat: 'Humidity: {point.x} % <br/> Temperature: {point.y} °C' 
      },
      series: [
          // Series data for humidity
      ]
  });

  colHiChart.value=Highcharts.chart('container1', {
      chart: { type: 'column', zoomType: 'x' },
      title: { text: 'Frequency Distribution Analysis', align: 'left' },
      // Add series data for Temperature, Humidity, and Heat Index variables
      series: [
          // Series data for Temperature
          // Series data for Humidity
          // Series data for Heat Index
      ]
  });

  scatHiChart.value=Highcharts.chart('container2', {
      chart: { type: 'scatter', zoomType: 'x' },
      title: { text: 'Temperature & Heat Index Correlation Analysis', align: 'left' },
      subtitle: { text: 'Visualize the relationship between Temperature and Heat Index as well as revealing patterns or trends in the data' },
      xAxis: {
          title: { text: 'Temperature' },
          labels: { format: '{value} °C' }
      },
      yAxis: {
          title: { text: 'Heat Index' },
          labels: { format: '{value} °C' }
      },
      tooltip: {
          pointFormat: 'Temperature: {point.x} °C <br/> Heat Index: {point.y} °C'
      },
      series: [
          // Series data for Temperature and Heat Index
      ]
  });

  scat2HiChart.value =Highcharts.chart('container3', {
      chart: { type: 'scatter', zoomType: 'x' },
      title: { text: 'Humidity & Heat Index Correlation Analysis', align: 'left' },
      subtitle: { text: 'Visualize the relationship between Humidity and Heat Index as well as revealing patterns or trends in the data' },
      xAxis: {
          title: { text: 'Humidity' },
          labels: { format: '{value} %' }
      },
      yAxis: {
          title: { text: 'Heat Index' },
          labels: { format: '{value} °C' }
      },
      tooltip: {
          pointFormat: 'Humidity: {point.x} % <br/> Heat Index: {point.y} °C'
      },
      series: [
          // Series data for Humidity and Heat Index
      ]
  });

};
onMounted(()=>{
  // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
  CreateCharts();

  Mqtt.connect(); // Connect to Broker located on the backend
 
  setTimeout( ()=>{
  // Subscribe to each topic
  Mqtt.subscribe("620156144");
  Mqtt.subscribe("620156144_pub");
  },3000)// THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
});


onBeforeUnmount(()=>{
  // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
  Mqtt.unsubcribeAll();
});

const updateLineCharts = async ()=>{
  if(!!start.value && !!end.value){
      // Convert output from Textfield components to 10 digit timestamps
      let startDate = new Date(start.value).getTime() / 1000;
      let endDate = new Date(end.value).getTime() / 1000;

      // Fetch data from backend
      const data = await AppStore.getAllInRange(startDate,endDate);

      // Create arrays for each plot
      let temperature = [];
      let heatindex = [];

      // Iterate through data variable and transform object to format recognized by highcharts
      data.forEach(row => {
      temperature.push({"x": row.timestamp * 1000, "y": parseFloat(row.temperature.toFixed(2)) });
      heatindex.push({ "x": row.timestamp * 1000,"y": parseFloat(row.heatindex.toFixed(2)) });
  });

  // Add data to Temperature and Heat Index chart
  tempHiChart.value.series[0].setData(temperature);
  tempHiChart.value.series[1].setData(heatindex);
  }
}

const updateCards = async () => {
  // Retrieve Min, Max, Avg, Spread/Range
  if(!!start.value && !!end.value){
  
      // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
      let startDate = new Date(start.value).getTime() / 1000;
      let endDate = new Date(end.value).getTime() / 1000;
      
      // 2. Fetch data from backend by calling the API functions
      const temp = await AppStore.getTemperatureMMAR(startDate,endDate);
      const humid = await AppStore.getHumidityMMAR(startDate,endDate);
      console.log(humid)
      
      temperature.max = temp[0].max.toFixed(1);
      //3. complete for min, avg and range
      temperature.min = temp[0].min.toFixed(1);
      temperature.avg = temp[0].avg.toFixed(1);
      temperature.range = temp[0].range.toFixed(1);
     
      //4. complete max, min, avg and range for the humidity variable
  }
}

const updateHistogramCharts = async () =>{
// Retrieve Min, Max, Avg, Spread/Range for Column graph
  if (!!start.value && !!end.value) {
      // 1. Convert start and end dates collected from TextFields to 10 digit timestamps
      // Subsequently, create startDate and endDate variables and then save the respective timestamps in these variables
      const startDate = Date.parse(start.value) / 1000;
      const endDate = Date.parse(end.value) / 1000;
      
      // 2. Fetch data (temp, humid, and hi) from the backend by calling the getFreqDistro API functions for each
      const temp = await AppStore.getFreqDistro("temperature", startDate, endDate);
      const humid = await AppStore.getFreqDistro("humidity", startDate, endDate);
      const hi = await AppStore.getFreqDistro("heatindex", startDate, endDate);
      
      // 3. Create an empty array for each variable (temperature, humidity, and heatindex)
      let temperature = [];
      let humidity = [];
      let heatindex = [];
      
      // 4. Iterate through the temp variable, which contains temperature data fetched from the backend
      // Transform the data to {"x": x_value, "y": y_value} format and then push it to the temperature array created previously
      temp.forEach(row => {
          temperature.push({ "x": row["_id"], "y": row["count"] });
      });
      
      // 5. Iterate through the humid variable, which contains humidity data fetched from the backend
      // Transform the data to {"x": x_value, "y": y_value} format and then push it to the humidity array created previously
      humid.forEach(row => {
          humidity.push({ "x": row["_id"], "y": row["count"] });
      });
      
      // 6. Iterate through the hi variable, which contains heat index data fetched from the backend
      // Transform the data to {"x": x_value, "y": y_value} format and then push it to the heatindex array created previously
      hi.forEach(row => {
          heatindex.push({ "x": row["_id"], "y": row["count"] });
      });
      
      // 7. Update series[0] for the histogram/Column chart with temperature data
      histogramChart.value.series[0].setData(temperature);
      
      // 8. Update series[1] for the histogram/Column chart with humidity data
      histogramChart.value.series[1].setData(humidity);
      
      // 9. Update series[2] for the histogram/Column chart with heat index data
      histogramChart.value.series[2].setData(heatindex);
  }
}
</script>


<style scoped>
/** CSS STYLE HERE */
Figure {
border: 2px solid black;
}

</style>