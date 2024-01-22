<script setup>
import { ref } from 'vue'
import { useRoute } from 'vue-router'

import Line from './Line.vue'

const route = useRoute();
const name = ref(route.params.name);
const words = ref([
    { shape: 'A', sound: 'aaa' },
    { shape: 'B', sound: 'bbb' },
    { shape: 'C', sound: 'ccc' }
]);
</script>

<template>
    <div>
        <!-- get from setup ref() -->
        <div>{{ name }}</div>

        <!-- get from this #app instance => vue.use(route) => vue.$route.params -->
        <div>{{ $route.params.name }}</div>

        <Line :words="words" />
    </div>
</template>

<style scoped></style>
