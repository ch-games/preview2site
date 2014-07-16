$(function() {
	var tmp = $('#betgamestv');
	$('#betgamestv').remove();
	$('.lottery_block').prepend(tmp);	
	if(window.addEventListener){
	    window.addEventListener('load',function(){postSize();},false); //W3C 	
	}else{
	    window.attachEvent('onload',function(){postSize();}); //IE
	}
	postSize(); 
});


function postSize(){
	var target = parent.postMessage ? parent : (parent.document.postMessage ? parent.document : undefined);
    if (typeof target != "undefined" && document.body.scrollHeight){ 
    	target.postMessage(document.getElementById("container").scrollHeight, "*");
	}    
    setTimeout(function(){postSize();} ,500);
}