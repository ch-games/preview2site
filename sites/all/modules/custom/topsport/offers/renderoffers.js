/*
 * Offers block over AJAX (node.js) renderer 
 */

var renderOffersObjectsIndex = 0;
function renderOffersBlock(ob, lang, type)
{
   var $ = jQuery;
   var obid = type.split('/').join('-') + (renderOffersObjectsIndex++);
   if(!ob){
    document.write('<div id="'+obid+'"></div>');
    ob = $('#' + obid);
   }
function renderBlockTabs(type,tabs)
{
    var t = '<ul id="'+type+'-list-conttrol" class="offers-list-control content-block-menu">', first = 0;
    
    for(i in tabs)
	{
        t += '<li id="pol_'+tabs[i].aid+'_popular" class="item'+(!(first++)?' first active':'')+'"><span class="icon"><img src="/sites/all/img/icons/area/'+tabs[i].aid+
	    '.png" alt="" width="18px"></span>'+tabs[i].name+'</li>';
	}      
      t += '</ul>';
      return t;
}
var loc = {eventNumber : ''} ; // t(', įvykio nr. @number', array('@number' => $offerData['#name']))
function formatDateTime(delta)
{
    
    var t = new Date(new Date().getTime() + delta * 1000),
	mo = t.getMonth()+1,
	d = t.getDate(),
	h = t.getHours(),
	m = t.getMinutes(),
	y = t.getYear() + 1900,
	ny = new Date().getYear() + 1900;
	
    if(delta > 24*3600)
    {
    
	return (y != ny ? y + '-' : '') + (mo < 10 ? '0' + mo : mo) + '-' + ( d < 10 ? '0' + d : d);
    }
    else
	{
	    if(delta < 3600)
		{
		    var mins = delta / 60;
		    return Drupal.t('už %min min').replace('%min', mins);
		}
		else return Drupal.t('šiandien') + ' ' + (h < 10 ? '0' + h : h) + ':' + ( m < 10 ? '0' + m : m);
	}	
	
}
function renderEvents(type, eventgroups, gametypes)
{
    var t = '';    
    
    for(ii in eventgroups)
    { 
	var oddeven = 1;
	events = eventgroups[ii];
	t += '<table class="odds control-offers-list" id="aid_'+events[0].aid+'_'+type+'"><tr header="1" class="aid_'+events[0].aid+'_'+type+' odd" style="display: table-row;"><th colspan="4"></th>'
	for(l=1;l<=6;l++)
	    if(gametypes[events[0].gtid]['cn'+l]) 
		t += '<th class="odd_title">'+gametypes[events[0].gtid]['cn'+l]+'</th>';
	t+='<th class="more"></th> </tr>';
	
	for(i in events)
	{
	    oddeven ^= 1;
	    id = events[i].eid + '-' + type;
	    odds = {};
	    for(j=1;j<=6;j++)
		if(events[i]['c'+j]) odds[j] = {code: j, title: '', rate : events[i]['c'+j]}; // TODO TITLE !!!
	    
	    t += '<tr id="e_'+id+'" class="'+ (odds.length > 1 ? "offers_line" : "") + '" title="' + events[i].category 
		+ (loc.eventNumber.split('@number').join(events[i].name)) + '">'
	    +'<td class="icon showalt"><img width="18px" alt="" src="/sites/all/img/icons/area/'+events[i].aid+'.png"></td>'
	    +'<td class="date">'+formatDateTime(events[i].timetostart)+'</td>'
	    +'<td class="number">'+events[i].name+'</td>'
	    +'<td class="odd_title" > ' + events[i].title + ' </td>';
	    for(j in odds)
	    {	    
		t += '<td id="e_' + events[i].eid + '_'  + odds[j].code + '" ' + (odds[j].rate > 1 ? 'onclick="Drupal.betAction(this, ' + events[i].eid + ', ' + odds[j].code + ');"' : '') + 
		' class="' + (odds[j].rate > 1 ? 'betaction' : '') + '">'  + (odds[j].rate > 1 ? '<span class="rate">' + odds[j].rate + '</span>' : '') + '</td>';
	    }
	    t+= '<td class="more" align="center">'
    //<?php print ($offerData['#nid'] ? l('+'.$offerData['#child_cnt'], 'node/'.$offerData['#nid'], array('query' => array('full' => 1))) : ''); ?>
    //<?php print offers_betradar_icon($offerData['#id']); ?>
	    t+= '</td></tr>';
	}
    }
	return t;
}


    $.getJSON('/njs/' + lang + '/' + type, function(d){
	var tt = type.split('/').join('-'),
	areas = {}, events = {};
	
	for(i=0;i<d.events.length;i++)
	{
	    var cat = d.events[i].category, cpos = cat ? cat.indexOf('»') : 0;
	    if(!cat) cat = '';
		if(!areas[d.events[i].aid]) areas[d.events[i].aid] = {aid: d.events[i].aid, name: cat.substr(0, cpos > 0 ? cpos: -1)};
		   
		if(!events[d.events[i].aid]) events[d.events[i].aid] = [];
		events[d.events[i].aid].push(d.events[i]);
	}	
	var out = renderBlockTabs(tt, areas);
	out += renderEvents(tt, events, d.gametypes);
	out += '</table>';
	$(ob).html(out);
	$('.control-offers-list', ob).hide();
	$('#aid_'+d.events[0].aid+'_'+tt, ob).show();
	var ptbl = $('#' + tt + '-list-conttrol', ob);
	$('li', ptbl).click(function(){
	    $('li', ptbl).removeClass('active');
	    $(this).addClass('active');
	    var si = $(this).attr('id').split('_');
	    si = si[1];
	    $('.control-offers-list', ob).hide();
	    $('#aid_'+si+'_'+tt, ob).show();
	});
    });
}

