jQuery(document).ready(function($){  

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
	type = $(clr).attr('stat-type'),
	p = $(clr).parent().parent(),	
	title = $(clr).attr('stat:evtitle') || $('.odd_title', p).text();
	if(type === 'popup'){
		$('#stat-dialog').remove();
		d = $('<div id="stat-dialog"><iframe src="'+uri+'"></iframe></div>').css({margin:0,padding:0,textAlign:'right',width:'100%',height:'100%'});		
		d.dialog({title:title, position:'center', top:null, left:null, width: 600, height: $(window).height()-100,resizable:false, modal: true});
		var pr = $('iframe', d).parent().parent('.ui-dialog');
		$('iframe', d).height((pr.height()-40));
	}else {
		window.open(uri);
	}
	
	//GA eventtracking
	trackEvent = ['_trackEvent'];
	trackEvent.push('StatLink');				
	trackEvent.push('event');
	trackEvent.push('url: '+ uri);
	_gaq.push(trackEvent);
}


$('#content').on('click','a.event-stats',function(){ 
	showstats(this);
}).on('click','img.more-icon', function(){
	if($(this).hasClass('inactive')) return;
	var p = $(this).parent();
	while(p.prop('nodeName') !== 'TR')
	{
		p = p.parent();
	}
	if(p.next().hasClass('altrow')) 
	{
		p.next().toggle();
	}	
});

	init_toggleGametypes();
  
});

function init_toggleGametypes(){	
	if(localStorage.getItem('hideGTFilterForm') == 'true'){
		var filter = jQuery('#offers_filter .filters-list form #edit-gametypes, #offers_filter #edit-submit-action');
		filter.addClass('collapse');
		toggleGametypes_controller('open');
	}
}

function toggleGametypes(){
	var filter = jQuery('#offers_filter .filters-list form #edit-gametypes, #offers_filter #edit-submit-action');
	if(filter.hasClass('collapse')){
		filter.removeClass('collapse');
		localStorage.setItem('hideGTFilterForm', false);
		toggleGametypes_controller('close');
	}else{
		filter.addClass('collapse');
		localStorage.setItem('hideGTFilterForm', true);
		toggleGametypes_controller('open');
	}
	return false;
}

function toggleGametypes_controller(state){
	var btn_icon = jQuery('.btn-toogle-gt-filter span');
	if(state === 'open'){
		btn_icon.removeClass('glyphicon-eye-close');
		btn_icon.addClass('glyphicon-eye-open');		
	}else {
		btn_icon.removeClass('glyphicon-eye-open');
		btn_icon.addClass('glyphicon-eye-close');	
	}
}

