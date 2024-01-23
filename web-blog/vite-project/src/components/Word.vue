<script setup>
import { defineProps } from 'vue'

const props = defineProps({
    shape: { type: String, required: true },
    sound: { type: String, default: '' }
});
</script>

<template>
    <span class="word" :before-content="props.sound">{{ props.shape }}</span>
</template>

<style scoped>
.word {
    --font-size: 18px;

    display: inline-block;
    position: relative;
    font-size: var(--font-size);
    min-width: var(--font-size);
    min-height: calc( var(--font-size) * 2 );
    margin: 0;
    border: 0;
    padding: 0;
}

.word::before {
    content: attr(before-content);
    position: absolute;
    font-size: calc( var(--font-size) * 0.5 );
    top: calc( var(--font-size) * -0.5 );
    left: calc( var(--font-size) * -0.5 );
    width: calc( var(--font-size) * 2 );
    opacity: 0.5;
}

.word:hover::before {
    opacity: 1;
}
</style>
