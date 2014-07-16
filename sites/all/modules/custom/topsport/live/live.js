(function ($) {
  function updateLives() {
    $.get(Drupal.settings.live.url, null, function(response){
      $(response.selector).html(response.data);
        setTimeout(function(){ updateLives(); }, 15000);
    });
  }
  function updateLivesBlock() {
    $.get(Drupal.settings.live.urlblock, null, function(response){
      $('#block-live-live .block-content').html(response.block.content);
        setTimeout(function(){ updateLivesBlock(); }, (60000 * 10));
    });
  }
  Drupal.behaviors.liveupdate = {
    attach: function (context, settings) {
      if(Drupal.settings.live.page){
        $('body', context).each(function () {
          var base = 'body-live';
          var element_settings = {};
          element_settings.url = Drupal.settings.live.url;
          element_settings.event = 'updateLive';
          element_settings.effect = 'fade';
          element_settings.progress = {};
          Drupal.ajax[base] = new Drupal.ajax(base, this, element_settings);
        });
        $('.live-event-data.expand:not(.processed)', context).addClass('processed').click(function(){
          live_less_action(this);
        });
        $('.live-less:not(.processed)', context).addClass('processed').click(function(){
          live_less_action(this);
        });
      }
    }
  };
  function live_less_action(elem){
    var base = 'body-live';
    $parent = $(elem).parents('.live-event');
    $parent.find('.live-event-details').slideToggle();
    $buttonMore = $parent.find('.live-more');
    $buttonMore.toggleClass('less');
    $parent.toggleClass('expanded');
    if($buttonMore.hasClass('less')){
      $buttonMore.html(Drupal.t('Suskleisti'));
    }else{
      $buttonMore.html(Drupal.t('Plačiau'));
    }
    var more = '';
    $('.live-event.expanded').each(function(){
      more += $(this) .attr('id') + ',';
    });
    Drupal.ajax[base].options.url = Drupal.settings.live.url + '&more=' + more;
  }
  function setUpdateTimer(){
    setTimeout(function(){
      $('body').trigger('updateLive');
      setUpdateTimer();
    }, 15000);
  }
  $(document).ready(function(){
    if(Drupal.settings.live.page){
      setUpdateTimer();
    }
    setTimeout(function(){ updateLivesBlock(); }, (60000 * 10));
  });
})(jQuery);
