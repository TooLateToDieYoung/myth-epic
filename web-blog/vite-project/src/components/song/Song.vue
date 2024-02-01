<script setup>
import { ref, onBeforeMount } from 'vue'

const menu = ref([]);

onBeforeMount(async function () {
    menu.value = await fetch('http://127.0.0.1:1204/song')
        .then(res => res.json())
        .catch(err => []);
});
</script>

<template>
    <div @click="clickHandler">
        Song
        <nav v-for="(val, idx) in menu" :key="idx">
            <router-link :to="'/song/lyrics/' + idx">
                [{{ idx }}] {{ val.title }}
            </router-link>
        </nav>
    </div>
</template>

<style scoped></style>