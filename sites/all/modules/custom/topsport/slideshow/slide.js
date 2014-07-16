(function ($) {
  
  requestAnimationFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame ||  window.webkitRequestAnimationFrame || window.msRequestAnimationFrame; 
  
  $.fn.fadeTransition = function(options) {
    var options = $.extend({pauseTime: 16000, transitionTime: 500}, options);
  //  var transitionObject;

    Trans = function(obj) {
      var current = 0;
      var nav = $("> ul.slide-controls li", obj);
      var els = $("> .slide", obj);
      var working = false;
	  var obwith = $(obj).width() / options.pauseTime;
      //var progressbar = $('#progressbar');
      var progress = 0, lastUpdate = 0, progressGoing = true;
      //progressbar.css('backgroundPosition', '-1025px 0');
      els.css("display", "none");

	  $(window).resize(function(){
		  obwith = $(obj).width() / options.pauseTime;
	  });

      showFirst();

      function showFirst() {
        $(els[current]).css("display", "block");
      }

      function transition(next) {
        if(working == false){
          working = true;
          //progressbar.css('backgroundPosition', '-1025px 0');	  
          if($(".tickercontainer", els[current]).hasClass('less')){
            $(".tickercontainer", els[current]).trigger('click');
          }
          $(els[current]).fadeOut(options.transitionTime, function(){
            $(nav[current]).removeClass('active');
            $(nav[next]).addClass('active');
            $(els[next]).fadeIn(options.transitionTime, function(){
              current = next;
              cue();
              working = false;
            });
          });
        }
      };

      function cue() {
        Start();
	//animate();
      };

      function animate()
      {
	if(progressGoing)
	{
	    var tm = new Date().getTime();
	    progress += tm - lastUpdate;
	    lastUpdate = tm;

	    //progressbar.css('backgroundPosition', (((obwith * progress) << 0) - 1025) + 'px 0' );
	    if(progress >= options.pauseTime)
	    {
		progress = 0;
		progressGoing = false;
		transition((current + 1) % els.length | 0);
	    }
	}
	  if(requestAnimationFrame) requestAnimationFrame(animate);
	  else setTimeout(animate, 50);
      }

      function Start() {
	  progressGoing = true;
	  //progressbar.css('backgroundPosition', '-1025px 0');
	  progress = 0;
	  lastUpdate = new Date().getTime();        
      };

      function Continue() {      
		lastUpdate = new Date().getTime();
		progressGoing = true;	
      };

      function Pause() {        
		progressGoing = false;
      };

      if ($("> .slide-controls li", obj).length > 1){
        $(obj).find("ul.slide-controls").each(function() {
          $(this).children().each( function(idx) {
          if ($(this).is("li"))
            $(this).click(function() {Pause();transition(idx);return false;})
          });
        });
        $(obj).mouseover(function(){
          Pause();
        }).mouseleave(function(){
          Continue();
        });
        cue();
	animate();
      }
      
    }

    return this.each(function() {
      var transitionObject = new Trans(this);
      transitionObject = null;
    });
  }
  $(document).ready(function(){
    $(".slide_area").fadeTransition();

    $(".slide_area .tickercontainer").mouseover(function(){
        elem = this;
        height = $('div', elem).height();
        $(elem).stop().animate({
          height: height
        }, 'fast').addClass('less');
    }).mouseleave(function(){
        elem = this;
        $(elem).stop().animate({
          height: "10"
        }, 'fast').removeClass('less');
    });

    $('.slide_area .slideshow-more').mouseover(function(){
      $(this).parent().find('.slideshow-more-title').show();
    }).mouseleave(function(){
      $(this).parent().find('.slideshow-more-title').hide();
    });
  });
})(jQuery);
