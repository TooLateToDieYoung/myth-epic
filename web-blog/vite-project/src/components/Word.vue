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
    min-width: var(--font-size);
    min-height: calc( var(--font-size) * 2 );
}

.word::before {
    position: absolute;
    content: attr(before-content);
    top: calc( var(--font-size) * -1 );
    left: 0;
    opacity: 0;
}

.word:hover::before {
    opacity: 1;
}
</style>
