<?php


/**
 * Render user campaings reward page
 * @return string
 */
function oddsaffiliate_user_campaings_reward_page() {
	global $user, $base_url, $base_path;
	drupal_set_title(t('Affiliate report'));
	$html = '';
	// show registered total count
	$registrations = db_query('SELECT  count(*) as total_registrations FROM {users} u RIGHT JOIN {affiliate_users} au ON u.uid = au.uid RIGHT JOIN {affiliate_campaigns} ac ON ac.campaign_id = au.campaign_id where au.campaign_id IN (SELECT campaign_id FROM {affiliate_campaigns} where uid = :uid)', array(':uid' => $user->uid ))->fetchObject();

	// show monthly profit & share level
	$profit = false /*oddsaffiliate_get_affiliate_profit_data($user->uid)*/;
	if ($profit != false){
		$html .= '<div class="share">'.
			'<strong>'. t('Total registrations') . ' : </strong>' . ($registrations->total_registrations) . '<br />'.
			'<strong>'. t('Your share') . ' : </strong>' . ($profit['Share']*100) . ' %'. '<br />'.
			'<strong>'. t('Monthly Profit') . ' : </strong>' . number_format($profit['Profit'], 2) .' '. $profit['Currency'] . '<br />'			
			.'</div>';
	}

	$campaigns = oddsaffiliate_get_affiliate_campaigns($user->uid);
	if ($campaigns){
		$rows   = array();
		foreach ($campaigns as $camp){
			$aff_url = $base_url.$base_path.'?'.CAMPAIGN_VAR.'='.$camp->campaign_id.'&utm_source=uid-'.$user->uid.'&utm_medium='.$camp->title.'&utm_campaign=Affiliates';
			$aff_url = '<input type="text" value="'.$aff_url.'" style="width:50px" />';
			$rows[] = array(
			date('Y-m-d H:i', strtotime($camp->created)),
			$camp->title,
			$camp->total_visits,
			$camp->total_clients,
			number_format(($camp->total_clients/($camp->total_visits > 0 ? $camp->total_visits : 1))*100, 2) . ' %',
			$aff_url
			);
		}
		$html .= theme('table', array('header'=> array('Created', 'Campaign Title', 'Uniques visits', 'Customers', 'Conversion', 'URL'), 'rows' => $rows));
	}

	$html .= render( drupal_get_form( 'oddsaffiliate_create_new_form' ) );
	//db_query('insert INTO {affiliate_campaigns} (campaign_id, title, uid) VALUES (111, \'Soccerway.com\', 44084) ');
	/*
	 $tmp = db_query('select * from {affiliate_campaigns}')->fetchAll();
	 var_dump($tmp);*/

	return $html;
}

/**
 * Render as XML the affiliates and wheirs clients page FOR TOP SPORT system
 * @return string
 */
function oddsaffiliate_get_affiliates_as_xml(){
	drupal_add_http_header('Content-Type', 'text/xml');
	drupal_add_http_header('charset', 'utf-8');

	$affilates = db_query('SELECT * FROM {users} WHERE uid IN (SELECT uid FROM {affiliate_campaigns} GROUP BY uid)')->fetchAll();
	if ($affilates){
		$xml  = '<affiliates>';
		foreach ($affilates as $affilate){
			$affilate->data = unserialize($affilate->data);
			$xml .= '<affiliate uid="'.$affilate->uid.'" firstname="'.(isset($affilate->data['firstname'])? $affilate->data['firstname'] : '').'" middlename="'.(isset($affilate->data['middlename'])? $affilate->data['middlename'] : '').'" lastname="'.(isset($affilate->data['surname'])? $affilate->data['surname'] : '').'" email="'.$affilate->mail.'" created="'.date('c', $affilate->created).'" currency="'.(isset($affilate->data['currency'])? $affilate->data['currency'] : '').'" sex="'.(isset($affilate->data['sex'])? $affilate->data['sex'] : 0).'" birthday="'.(isset($affilate->data['birthday'])? $affilate->data['birthday'] : '').'">';
			$affilate_clients = oddsaffiliate_get_affiliate_users($affilate->uid);
			if ($affilate_clients){
				foreach ($affilate_clients as $client){
					$client->data = unserialize($client->data);
					$xml .= '<customer uid="'.$client->uid.'" campaign_id="'.$client->campaign_id.'" campaign_title="'.$client->title.'" tsuid="'.(isset($client->data['tsuid'])? $client->data['tsuid'] : '').'" created="'.date('c', $client->created).'" ip_address="'.$client->ip.'" geo_location="'.geoip_country_code($client->ip).'" />';
				}
			}
			$xml .= '</affiliate>';
		}
		$xml .= '</affiliates>';
	}
	print $xml;
}

