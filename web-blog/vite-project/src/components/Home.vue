<script setup>
import { ref, onBeforeMount } from 'vue'

const menu = ref({});
const clickHandler = ref( async function () {
    await Promise.all([
        setTimeout(console.log, 1000, '>'),
        setTimeout(console.log, 2000, '<')
    ]);

    console.log('mythepic');
});

onBeforeMount( async function () { 
    await setTimeout(console.log, 5000, 'test');
    menu.value['myth'] = true;
    menu.value['epic'] = 6666;
});
</script>

<template>
    <div @click="clickHandler">
        Home
        <nav v-for="(val, key, idx) in menu" :key="key">[{{ idx }}] {{ key }} : {{ val }}</nav>
    </div>
</template>

<style scoped>

</style>