<script setup>
// import { ref } from 'vue'
// import { useRoute } from 'vue-router'

import Line from './Line.vue'

import song from '../assets/song.json'

// const route = useRoute();
// const name = ref(route.params.name);
</script>

<template>
    <div>
        <!-- get from setup ref() 
        <div>{{ name }}</div>
        -->

        <!-- get from this #app instance => vue.use(route) => vue.$route.params 
        <div>{{ $route.params.name }}</div>
        -->

        <div class="title">{{ song.name }} - {{ song.singer }}</div>
        <div class="lyrics">
            <Line v-for="(value, index) in song.lyrics" :key="index" :line="value.line" :sign="value.sign" />
        </div>
    </div>
</template>

<style scoped>
.title {
    font-size: 24px;
    margin-bottom: 36px;
}
</style>
