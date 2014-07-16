$(function() {
    if ($('a.game-title').hasClass('logo_url_en')) {
        $('a.game-title').attr('href', 'http://m.bettopsport.com/');
    }
    if ($('a.game-title').hasClass('logo_url_ru')) {
        $('a.game-title').attr('href', 'http://m.bettopsport.ru/');
    }
});