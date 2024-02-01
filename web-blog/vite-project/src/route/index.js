import { createRouter, createWebHistory } from 'vue-router'

export default createRouter({
    history: createWebHistory(),
    routes: [
        {
            path: '/home',
            name: 'Home',
            component: () => import('../components/Home.vue')
        },
        {
            path: '/',
            redirect: '/Home'
        },
        {
            path: '/song',
            name: 'Song',
            component: () => import('../components/song/Song.vue')
        },
        {
            path: '/song/lyrics/:id',
            name: 'Lyrics',
            component: () => import('../components/song/Lyrics.vue')
        }
    ]
});
