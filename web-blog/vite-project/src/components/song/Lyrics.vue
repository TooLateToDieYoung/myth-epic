<script setup>
import { ref, onBeforeMount } from 'vue'
import { useRoute } from 'vue-router'

import Line from '../Line.vue'

const route = useRoute();

const song = ref({});

const getSong = async function (id) {
    try {
        const res = await fetch('http://127.0.0.1:1204/song?id=' + id);
        if (!res.ok) {
            throw 'fetch failed'
        }

        return await res.json();
    }
    catch (err) {
        return { name: "N/A", singer: "N/A", lyrics: [] };
    }
}

onBeforeMount(async function () {
    song.value = await getSong(route.params.id);
});
</script>

<template>
    <div>
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
