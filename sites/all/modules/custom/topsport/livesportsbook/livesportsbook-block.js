jQuery(document).ready(function() {
	jQuery("#block-livesportsbook-livesportsbook li.live-match").hover(
		function() {
			jQuery(this).bind('hover', function() {				
		        var ob = jQuery(this).find('span.title');
		        var tw = ob.width();
		        var ww = ob.parent().width();
		        if(tw > ww ) {
			        ob.css({ left: 0 });
			        ob.animate({ left: -ww }, 9000, 'linear', function() {
			            ob.trigger('hover');
			        });
		       }
		    }).trigger('hover');
		},
		function() {
			jQuery(this).find('span.title').trigger('hover').stop(true);
		}
	);
});