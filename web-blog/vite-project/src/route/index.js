import { createRouter, createWebHistory } from 'vue-router'

export default createRouter({
    history: createWebHistory(),
    routes: [
        {
            path: '/Home',
            name: 'Home',
            component: () => import('../components/Home.vue')
        },
        {
            path: '/',
            redirect: '/Home'
        },
        {
            path: '/Lyrics/:name',
            component: () => import('../components/Lyrics.vue')
        }
    ]
});
