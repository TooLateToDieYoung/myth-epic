<script setup>
import { defineProps } from 'vue'

import Word from './Word.vue'

const props = defineProps({
    line: { type: String, required: true },
    sign: { type: Object, default: { /* "idx": "sign" */ } }
});

const searchSign = function (idx) {
    const key = idx.toString();
    return key in props.sign ? props.sign[key] : '';
};
</script>

<template>
    <div class="line">
        <Word v-for="(word, idx) in props.line" :key="idx" :shape="word" :sound="searchSign(idx)" />
    </div>
</template>

<style scoped></style>
