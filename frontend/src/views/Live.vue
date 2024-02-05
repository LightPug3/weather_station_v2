<template>
    <v-container fluid background-color="surface" align="center">
        <!-- row 1 -->
        <v-row style="max-width: 1200px;">
            <!-- col 1 -->
            <v-col cols="9" >
                <figure class="highcharts-figure">
                    <div id="container">
                        <!-- Temperature graph -->
                    </div>
                </figure>
            </v-col>

            <!-- col 2 -->
            <v-col cols="3">
                <v-card></v-card>
            </v-col>
        </v-row>

        <!-- row 2 -->
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
 
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  


// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
});


</script>


<style scoped>
/** CSS STYLE HERE */


</style>
  