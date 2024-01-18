<script setup>
import { ref } from 'vue'
import { useRoute } from 'vue-router'

const route = useRoute();
const name = ref(route.params.name);

</script>

<template>
    <div>
        <!-- get from setup ref() -->
        <div>{{ name }}</div>

        <!-- get from this #app instance => vue.use(route) => vue.$route.params -->
        <div>{{ $route.params.name }}</div>
    </div>
</template>

<style scoped>

</style>
