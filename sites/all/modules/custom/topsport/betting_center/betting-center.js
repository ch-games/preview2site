jQuery(document).ready(function(){
	var myOptions = {
		zoom: Drupal.settings.betting_center_map['zoom'],
		center: new google.maps.LatLng(Drupal.settings.betting_center_map['loc'][0], Drupal.settings.betting_center_map['loc'][1]),
		mapTypeId: google.maps.MapTypeId.ROADMAP
	};
	geocoder = new google.maps.Geocoder();
	var map = new google.maps.Map(document.getElementById("map"), myOptions);
	setMarkers(map, Drupal.settings.betting_center);	
	function setMarkers(map, locations) {
		var image = new google.maps.MarkerImage(Drupal.settings.map_icon, new google.maps.Size(86, 51), new google.maps.Point(0,0), new google.maps.Point(43, 51));
		for (var i = 0; i < locations.length; i++) {
			var loc = locations[i];
			var myLatlng = new google.maps.LatLng(loc[0], loc[1]);
			var marker = new google.maps.Marker({
				map: map, 
				icon: image,
				position: myLatlng,
				url: loc[2]
			});
			google.maps.event.addListener(marker, 'click', function() {
				window.location.href = this.url;
			});
		};
	}
});
