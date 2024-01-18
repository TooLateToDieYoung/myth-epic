import { createApp } from 'vue'
import './style.css'
import App from './App.vue'
import route from './route/index.js'

createApp(App).use(route).mount('#app')
