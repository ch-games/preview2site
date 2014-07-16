jQuery(document).ready(function($){
  $('#filter-toggle').click(function(){
    $(this).parent().find('li').removeClass('active');
    $(this).addClass('active');
    $('.filters-list').toggle();
  });
/*
  $('.offers_line').each(function(){
    var maxId = ''; var maxVal = 0;
    var minId = ''; var minVal = 9999;
    $('td.betaction', this).each(function(){
      if(Drupal.convertToFloat($(this).html()) > maxVal){
        maxVal = Drupal.convertToFloat($(this).html());
        maxId = '#' + $(this).attr('id');
      }
      if(Drupal.convertToFloat($(this).html()) < minVal){
        minVal = Drupal.convertToFloat($(this).html());
        minId = '#' + $(this).attr('id');
      }
    });
    $(maxId).addClass('max_value');
    $(minId).addClass('min_value');
  });
 */
  $('#offers-mark-selection select').change(function(){
    $('td.betaction').removeClass('marked');
    var action = $(this).val();
    if(action == 'min'){
      $('.offers_line').each(function(){
        var minId = ''; var minVal = 9999;
        $('td.betaction', this).each(function(){
          if(Drupal.convertToFloat($('.rate', this).html()) < minVal){
            minVal = Drupal.convertToFloat($('.rate', this).html());
            minId = '#' + $(this).attr('id');
          }
        });
        $('td.betaction', $(this).next('tr.altrow')).each(function(){
          if(Drupal.convertToFloat($('.rate', this).html()) < minVal){
            minVal = Drupal.convertToFloat($('.rate', this).html());
            minId = '#' + $(this).attr('id');
          }
        });
        $(minId).addClass('marked');
      });
    }else if(action == 'max'){
      $('.offers_line').each(function(){
        var maxId = ''; var maxVal = 0;
        $('td.betaction', this).each(function(){
          if(Drupal.convertToFloat($('.rate', this).html()) > maxVal){
            maxVal = Drupal.convertToFloat($('.rate', this).html());
            maxId = '#' + $(this).attr('id');
          }
        });
        $('td.betaction', $(this).next('tr.altrow')).each(function(){
          if(Drupal.convertToFloat($('.rate', this).html()) > maxVal){
            maxVal = Drupal.convertToFloat($('.rate', this).html());
            maxId = '#' + $(this).attr('id');
          }
        });
        $(maxId).addClass('marked');
      });
    }
  });

   $('#offers-area-select select').change(function(){
     var area = $(this).val();
     if(area != 0){
       $('.offers-list-area').addClass('hide');
       $('#block-offers-area-' + area).removeClass('hide');
     }else{
       $('.offers-list-area').removeClass('hide');
    }
  });
  
  
  /// 
function showstats(clr)
{
	var uri = $(clr).attr('stat:uri'),
	p = $(clr).parent().parent(),	
	title = $(clr).attr('stat:evtitle') || $('.odd_title', p).text();
	
	$('#stat-dialog').remove();
	d = $('<div id="stat-dialog"><iframe src="http://live.topsport.lt/'+uri+'"></iframe></div>').css({margin:0,padding:0,textAlign:'right',width:'100%',height:'100%'});		
	d.dialog({title:title, position:'center', top:null, left:null, width: 570, height: 550,resizable:false, modal: true /* resize:function(ev,ui){	
		var tis = $(this), pr = $('iframe',tis).parent();	
		$('iframe', tis).width( (pr.width() << 0) - 6 ).height( (pr.height() << 0) - 10);
	}*/});
	var pr = $('iframe', d).parent();
	$('iframe', d).width((pr.width() << 0) - 6 ).height((pr.height() << 0) - 10);
}
console.log('reimplement: $(a.event-stats).live');
/*$('a.event-stats').live('click',function(){
	showstats(this);
});*/
  
});


