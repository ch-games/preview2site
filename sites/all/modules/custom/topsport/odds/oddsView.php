<?php
class OddsView {
	/** templates controller **/
	private $default_gametypes = '';
	private $offersData = array();
	private $gametypes = array();

	
	public function __construct($offerData = null, $gametypes = null)
	{
		
		$this->offersData = $offerData;
		$this->gametypes = $gametypes;
	}
	
	public function renderArea($aid)
	{
		
		return $this->renderTemplate('odds-list-area.tpl.php', $this->data);
	}
	
	public function renderResults($results)
	{	
		$areaHtml = '';
        if(count($results)){
            foreach ($results as $area)
            {
                $aid = &$area['id'];
                $countryHtml = '';
                foreach ($area['children'] as $country)
                {
                    $cid = &$country['id'];
                    $moduleHtml = '';
                    foreach ($country['children'] as $module)
                    {
                        $mid = &$module['id'];
                        $gametypeHtml = '';
                        foreach ($module['children'] as $gametype => $events)
                        {
                            $eventsHtml = '';
                            foreach ($events as $event)
                            {   
                                $event['path'] = '/oddresults/'.$aid.'/'.$cid.'/'.$mid;
                                $eventsHtml .= $this->renderTemplate('oddsresults/templates/results-event.tpl.php', $event);
                            }
                            $gametypeHtml .= $this->renderTemplate('oddsresults/templates/results-gametype.tpl.php', array('eventsHtml'=>$eventsHtml, 'title'=>$gametype));
                        }
                        $module['title'] = $this->getLanguageIcon($cid, $country['title']). $module['title'].( (int)$cid > 0 ? '<span class="odds-midcountry icon-country-'.$cid.'">'.$country['title'].'</span>' : '');
                        $moduleHtml .= $this->renderTemplate('oddsresults/templates/results-module.tpl.php', array('gametypesHtml'=>$gametypeHtml, 'title'=>$module['title'], 'class'=>''));
                    }                
                    $countryHtml .= $this->renderTemplate('oddsresults/templates/results-country.tpl.php', array('modulesHtml'=>$moduleHtml, 'title'=>$country['title'], 'block_html_id'=>'block-offers-area-'. $aid . '-country-' .$cid, 'area_id'=>$aid, 'country_id'=>$cid, 'classes'=>'offers-list-country offers_country'));
                }
                $areaHtml .= $this->renderTemplate('oddsresults/templates/results-area.tpl.php', array('countriesHtml'=>$countryHtml, 'title'=>$area['title'], 'block_html_id'=>'block-offers-area-' .$aid, 'classes'=>'offers-list-area offers_area', 'aid'=>$aid));
            }
        } else {
            $areaHtml = '<div class="alert alert-warning">'.t('Rezultatų nėra...').'</div>';
        }
		return $areaHtml;
	}
	
	public function renderMatch($type, $euroview)
	{
		$data = $this->offersData;
		$offerHtml = '';
		if(isset($data['area']) && is_array($data['area'])){
			foreach ($data['area'] as $aid => $area)
			{
				$areaHtml = '';
				foreach ($area['country'] as $cid => $country)
				{
					$countryHtml = '';
					foreach ($country['module'] as $mid => $module)
					{
						$moduleHtml = '';
						foreach ($module['gameTypes'] as $gameType)
						{
							$gameTypeHtml = '';
							foreach ($gameType['offerlist'] as $offerlist)
							{
								if($euroview)
								{
									$gameTypeHtml .= $this->renderTemplate('templates/odds-list-offer-europeview.tpl.php', array('offerData' => $offerlist, 'filtered' => false, 'eid' => true, 'path'=>'odds/' . $type . '/' . $aid .'/' . $cid . '/' . $mid . '/' ));
								}
								else
								{									
									$gameTypeHtml .= $this->renderTemplate('templates/odds-list-offer.tpl.php', array('offerData' => $offerlist, 'eid' => true, 'path'=>'odds/' . $type . '/' . $aid .'/' . $cid . '/' . $mid . '/' ));
								}
							}
							
							if($euroview)		
							{
                                //var_dump($gameType['collapsed']);
								$moduleHtml .= $this->renderTemplate('templates/odds-list-gametype-europeview.tpl.php', array('title'=>$gameType['title'],'gameType_id'=>$gameType['id'],'content'=>$gameTypeHtml, 'gameTypeCollapsed' => /*$gameType['collapsed']*/ FALSE));
							}
							else
							{
								$cofnames = array();
								for($i=0;$i<6 && strlen($this->gametypes[$gameType['id']][$i]);$i++)
									$cofnames[$i] = $this->gametypes[$gameType['id']][$i];
								$moduleHtml .= $this->renderTemplate('templates/odds-list-gametype.tpl.php', array('title'=>$gameType['title'],'coefNames'=>$cofnames,'gameType_id'=>$gameType['id'],'content'=>$gameTypeHtml));
							}
						}
						//$countryHtml .= $this->renderTemplate('templates/odds-list-module.tpl.php', array('module_id'=>$mid, 'title'=>$this->getModuleIcon($mid, $module['title']).$module['title'], 'content'=>$moduleHtml, 'class'=>''));
						$moduleTitle = $module['title'];
					}
					//$areaHtml .= $this->renderTemplate('templates/odds-list-country.tpl.php', array('title'=>$country['title'], 'content'=>$countryHtml, 'block_html_id'=>'block-offers-area-'. $aid . '-country-' .$cid, 'area_id'=>$aid, 'country_id'=>$cid, 'classes'=>'offers-list-country block offers_country remember expanded'));
					$countryTitle = $country['title'];
				}
				//$offerHtml .= $this->renderTemplate('templates/odds-list-area.tpl.php', array('title'=>$area['title'], 'content'=>$areaHtml, 'block_html_id'=>'block-offers-area-' .$aid, 'classes'=>'offers-list-area block offers_area remember expanded'));
				$areaTitle = $area['title'];
			}
			$finalHtml = $this->renderTemplate('templates/odds-match-list.tpl.php', array( 'content'=>$moduleHtml, 'area'=>$areaTitle, 'country'=>$countryTitle, 'module'=>$moduleTitle));
		} else {
			$finalHtml =  t('Patikrinkite adresą...');
		}
		return $finalHtml;
	}

	public function renderSuperBet($type, $euroview)
	{
		$data = $this->offersData;
		$offerHtml = '';
		if(isset($data['area']) && is_array($data['area'])){
			foreach ($data['area'] as $aid => $area)
			{
				$areaHtml = '';
				foreach ($area['country'] as $cid => $country)
				{
					$countryHtml = '';
					foreach ($country['module'] as $mid => $module)
					{
						$moduleHtml = '';
						foreach ($module['gameTypes'] as $gameType)
						{
                            $moduleHtml .= '<div class="offer">';
							if(isset($gameType['topbet'])){
                                $moduleHtml .= $this->renderTemplate('templates/blocks/superbet-comb-buttons.tpl.php', array('topbets'=> $gameType['topbet']));
                            }
                            $moduleHtml .= '<table class="odds">';
                            $itteration = 0;
                            $total = count($gameType['offerlist']);
							foreach ($gameType['offerlist'] as $offerlist)
							{
								if($euroview)
								{
									$moduleHtml .= $this->renderTemplate('templates/odds-list-offer-europeview.tpl.php', array('offerData' => $offerlist, 'filtered' => isset($filtered) ? $filtered : false, 'eid' => null, 'path'=>'odds/all/' . $aid .'/' . $cid . '/' . $mid . '/','itteration'=> $itteration, 'total' => $total ));
								}
								else
								{									
									$moduleHtml .= $this->renderTemplate('templates/odds-list-offer.tpl.php', array('offerData' => $offerlist, 'eid' => null, 'path'=>'/odds/all/' . $aid .'/' . $cid . '/' . $mid . '/' ));
								}
                                $itteration++;
							}
                            $moduleHtml .= '</table></div>';
						}
                        $module_country = ( (int)$cid > 0 ? '<span class="odds-midcountry icon-country-'.$cid.'">'.$country['title'].'</span>' : '');
                        $sport_icon = '<span class="tssporticon tssporticon-s'.$aid.'"></span>';
						$countryHtml .= $this->renderTemplate('templates/odds-list-module.tpl.php', array('module_id'=>$mid, 'title'=> $sport_icon.$module['title'].$module_country, 'content'=>$moduleHtml, 'class'=>''));
					}
					$areaHtml .= $this->renderTemplate('templates/odds-list-country.tpl.php', array('title'=>$country['title'], 'content'=>$countryHtml, 'block_html_id'=>'block-offers-area-'. $aid . '-country-' .$cid, 'area_id'=>$aid, 'country_id'=>$cid, 'classes'=>'offers-list-country offers_country'));
				}
				$offerHtml .= $this->renderTemplate('templates/odds-list-area.tpl.php', array('title'=>$area['title'], 'content'=>$areaHtml, 'block_html_id'=>'block-offers-area-' .$aid, 'classes'=>'offers-list-area block offers_area remember expanded'));
			}
			$finalHtml = $this->renderTemplate('templates/odds-list.tpl.php', array( 'content'=>$offerHtml));
		} else {
			$finalHtml =  t('Patikrinkite adresą...');
		}
		return $finalHtml;
	}


	public function renderBlock($type, $euroview, $filtered = false, $show_sport = false)
	{
		$data = $this->offersData;
		$offerHtml = '';
		if(isset($data['area']) && is_array($data['area'])){
			foreach ($data['area'] as $aid => $area)
			{          
				$areaHtml = '';
				foreach ($area['country'] as $cid => $country)
				{
					$countryHtml = '';
					foreach ($country['module'] as $mid => $module)
					{
						$moduleHtml = '';   
                        $gts_count = 0;
						foreach ($module['gameTypes'] as $gameType)
						{
							$gameTypeHtml = '';                                                     
                            $gameTypeCollapsed = FALSE; 
                            $min_time = 0;
                            foreach ($gameType['offerlist'] as $offerlist)
							{
                                $is_longterm = ($offerlist['name'] >= 60000 && $offerlist['name'] <= 99999);
                                if( ($min_time == 0 && $is_longterm)  || ($min_time > $offerlist['date'] && $is_longterm)){ $min_time = $offerlist['date'];}
								if($euroview)
								{
									$gameTypeHtml .= $this->renderTemplate('templates/odds-list-offer-europeview.tpl.php', array('offerData' => $offerlist, 'filtered' => $filtered, 'eid' => null, 'path'=>'odds/all/'. $aid. '/' . $cid . '/' .$mid .'/','type' => $type ));
								}
								else
								{									
									$gameTypeHtml .= $this->renderTemplate('templates/odds-list-offer.tpl.php', array('offerData' => $offerlist, 'eid' => null, 'path'=>'odds/all/'. $aid. '/' . $cid . '/' .$mid .'/' ));
								}
							}
							if(!$filtered && $type == 'all' && strtotime($min_time) > strtotime('+ 4 weeks')){
                                if($gts_count > 0){ // Only first from long term show expanded
                                    $gameTypeCollapsed = true;                                     
                                }
                                $gts_count++;
                            }
							if($euroview)		
							{
								$moduleHtml .= $this->renderTemplate('templates/odds-list-gametype-europeview.tpl.php', array('title'=>$gameType['title'],'gameType_id'=>$gameType['id'],'content'=>$gameTypeHtml, 'gameTypeCollapsed' => $gameTypeCollapsed));
							}
							else
							{
								$cofnames = array();
								for($i=0;$i<6 && strlen($this->gametypes[$gameType['id']][$i]);$i++)
									$cofnames[$i] = $this->gametypes[$gameType['id']][$i];
								$moduleHtml .= $this->renderTemplate('templates/odds-list-gametype.tpl.php', array('title'=>$gameType['title'],'coefNames'=>$cofnames,'gameType_id'=>$gameType['id'],'content'=>$gameTypeHtml));
							}                            
						}
                        $module_country = ( (int)$cid > 0 ? '<span class="odds-midcountry icon-country-'.$cid.'">'.$country['title'].'</span>' : '');
                        $lang_icon = $this->getLanguageIcon($cid, $country['title']). ' ';
						$countryHtml .= $this->renderTemplate('templates/odds-list-module.tpl.php', array('module_id'=>$mid, 'title'=> $lang_icon.$module['title'].$module_country, 'content'=>$moduleHtml, 'class'=>''));
					}
					$areaHtml .= $this->renderTemplate('templates/odds-list-country.tpl.php', array('title'=>$country['title'], 'content'=>$countryHtml, 'block_html_id'=>'block-offers-area-'. $aid . '-country-' .$cid, 'area_id'=>$aid, 'country_id'=>$cid, 'classes'=>'offers-list-country offers_country'));
				}
				$offerHtml .= $this->renderTemplate('templates/odds-list-area.tpl.php', array('title'=>$area['title'], 'content'=>$areaHtml, 'block_html_id'=>'block-offers-area-' .$aid, 'classes'=>'offers-list-area block offers_area', 'show_header' => $show_sport, 'aid' => $aid, 'area_path' => 'odds/'.$type.'/'.$aid ));
			}
			$finalHtml = $this->renderTemplate('templates/odds-list.tpl.php', array( 'content'=>$offerHtml));
		} else {
			$finalHtml =  t('Patikrinkite adresą...');
		}
		return $finalHtml;
	}

	protected function getModuleIcon($mid, $title = '')
	{

		$ext = null;
		if(file_exists(DRUPAL_ROOT.'/sites/all/img/icons/module_png/'.$mid.'.png')){
		  $ext = 'png';
		}elseif(file_exists(DRUPAL_ROOT.'/sites/all/img/icons/module_gif/'.$mid.'.gif')){
		  $ext = 'gif';
		}
		if($ext){
		  $variables = array(
			'path' => '/sites/all/img/icons/module_'.$ext.'/'.$mid.'.'.$ext,
			'alt' => '',
			'title' => $title,
			'width' => null,
			'height' => null,
			'attributes' => array('class' => 'area-icon', 'style' => 'height: 30px;'),
		  );
		  return theme_image($variables);
		}else{
		  $variables = array(
			'path' => '/sites/all/img/icons/module-icon.png',
			'alt' => '',
			'title' => $title,
			'width' => null,
			'height' => null,
			'attributes' => array('class' => 'module-icon'),
		  );
		  return theme_image($variables);
		}
		return '';

	}
    
    	protected function getLanguageIcon($cid, $title = '')
	{

		$ext = null;
        $path = '/front/assets/flags/iso/24_ts/';
        
		if(file_exists(DRUPAL_ROOT.$path.$cid.'.png')){
		  $ext = 'png';
		}
		if($ext){
		  $variables = array(
			'path' => $path.$cid.'.'.$ext,
			'alt' => $title,
			'title' => $title,
			'width' => null,
			'height' => null,
			'attributes' => array('class' => 'icon-country pull-left'),
		  );
		  return theme_image($variables);
		}
		return '';

	}
    
    	public static function getCountryIcon($cid, $title = '')
	{

		$ext = null;
        $path = '/front/assets/flags/iso/24_ts/';
        
		if(file_exists(DRUPAL_ROOT.$path.$cid.'.png')){
		  $ext = 'png';
		}
		if($ext){
		  $variables = array(
			'path' => $path.$cid.'.'.$ext,
			'alt' => $title,
			'title' => $title,
			'width' => null,
			'height' => null,
			'attributes' => array('class' => 'icon-country'),
		  );
		  return theme_image($variables);
		}
		return '';

	}


	public function getCountries(){

	}

	public function getLeagues(){

	}

	public function getMatches(){

	}
	
	protected function renderTemplate($template, $data)
	{		
		extract($data);
		ob_start();
		include $template;
		return ob_get_clean();
	}
    
    public static function renderBtnBet($cor_pattern, $odds_value, $event_id = null, $choice_id = null, $ga_category = 'default', $tooltip = false)
    {
        $attrs['id'] = $attrs['data-eid'] = 'e_'.$event_id.'_'.$choice_id;
        $attrs['class'] = 'btn btn-bet'.($odds_value > 1 ? '' : ' disabled');
        $ga = '\''.$ga_category.'\'';
        if($tooltip){
            $attrs['data-toggle'] = 'tooltip';
            $attrs['title'] = $tooltip;
        }
        $attrs['onclick'] = 'Drupal.betAction(this,' . $event_id . ',' . $choice_id . ','.$ga.');';
        $html = '<a '.self::renderAttributes($attrs).'>'
                    . '<div class="pull-right badge rate">'.($odds_value > 1 ? OddsViewHelper::formatOddsValue($odds_value) : '<span class="glyphicon glyphicon-lock"></span>').'</div>'
                    .'<div class="condition">'.$cor_pattern.'</div>'
                . '</a>';
        return $html;
    }
    
    
    private static function renderAttributes($attributes = array()){
        $output = '';
        if(count($attributes))
        foreach($attributes as $key => $value){
            $output .= $key.'="'.$value.'" ';
        }
        return $output;
    }
}