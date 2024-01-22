<script setup>
import { defineProps } from 'vue'

import Word from './Word.vue'

const props = defineProps({
    words: { type: Array, required: true }
});
</script>

<template>
    <div class="line">
        <Word v-for="(word, index) in props.words" :key="index" :shape="word.shape" :sound="word.sound" />
    </div>
</template>

<style scoped></style>
