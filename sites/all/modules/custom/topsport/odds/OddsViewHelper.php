<?php
/**
 * This class helps OddsView format data
 *
 * @author Irmantas
 */
class OddsViewHelper {
    static private $sports_alias = array(
      1 => 'football',
      2 => 'basketball'
    );
    
    static private $match_info = array(
      '1/8F' => 'Aštuntfinalis',
      '1/4F' => 'Ketvirtfinalis',
      '1/2F' => 'Pusfinalis',
      'FIN' => 'Finalas',
      1 => 'Pirmos rungtynės',
      2 => 'Antros rungtynės',
      3 => 'Trečios rungtynės',
      4 => 'Ketvirtos rungtynės',
      5 => 'Penktos rungtynės',
      6 => 'Šeštos rungtynės',
      7 => 'Septintos rungtynės',
      '3.V' => 'Rungtynės dėl 3-os vietos', 
      'Q' => 'Kvalifikacija'
    );
    
    static private $stats_domains = array(
      'h2h' => 'http://live.topsport.lt',
      'mc' => 'http://mc.topsport.lt',
      'tennismc' => 'http://tennismc.topsport.lt'
    );


    public static function sportAlias($sport_id){
        if(isset(self::$sports_alias[$sport_id])){
            $sport_id = self::$sports_alias[$sport_id];
        }
        return $sport_id;
    }
    
    /**
     * Odds value fixing if value so big and 
     * @param type $value
     * @return type
     */
    public static function formatOddsValue($value){
        if($value >= 5){
            $fraction = $value - floor($value); 
            if($fraction == 0){
                $value = (int)$value;
            }
        }
        return $value;
    }
    
    public static function formatOddsDate($time) {
        if(!is_numeric($time)){
            $time = strtotime($time);
        }        
        $tomorrow = strtotime('tomorrow');
        $onemoreday = strtotime('+1 day', $tomorrow);
        $dif = $time - REQUEST_TIME;

        $icon_variables = array(
          'path' => '/sites/all/img/icons/siandien.gif',
          'alt' => '',
          'title' => t('šiandien'),
          'width' => null,
          'height' => null,
          'attributes' => array('class' => 'today-icon'),
        );
        if ($dif < 3600 AND $dif > 0){
            /* avoiding confusing if we will use caching so we cand recalc every minute this date
             * return '<span class="soon">'.t('už %min min', array('%min' => round($dif / 60, 0))).'</span>';*/
            return '<span class="soon">'.t('šiandien') . ' ' . format_date($time, 'custom', 'H:i').'</span>';
        } elseif ($time < $tomorrow) {
            return '<span class="today">'.t('šiandien') . ' ' . format_date($time, 'custom', 'H:i').'</span>';
        } elseif ($time < $onemoreday){
            return '<span class="tomorrow">'.t('rytoj') . ' ' . format_date($time, 'custom', 'H:i').'</span>';
        }
        if (date('Y') != date('Y', $time)) {
            return format_date($time, 'custom', 'Y-m-d H:i');
        }
        return format_date($time, 'custom', 'm-d H:i');
    }    
    
    public static function formatResultsDate($time) {
        if (date('Y') != date('Y', $time)) {
            return format_date($time, 'custom', 'Y-m-d');
        }
        return format_date($time, 'custom', 'm-d');
    }
    
    public static function linkMore($count, $url){
        return '<a href="/'.$url.'"'.($count == 0 ? ' class="inactive"': '').'>+'.($count == 0 ? '00': $count).'</a>';
    }
    
    public static function isLongTermOffer($event_id){
        return ($event_id >= 60000 && $event_id <= 99999);
    }
    
    public static function getMatchExtension($extension){
        if($extension){
            if(isset(self::$match_info[$extension])){
                return self::$match_info[$extension];
            }
        }
        return null;
    }
    
    public static function getMatchTitle($matchObj){
        $title = '';
        if(isset($matchObj['item1name']) && isset($matchObj['item2name'])){
            $title = $matchObj['item1name'] . ' - ' . $matchObj['item2name'];
        }
        return $title;
    }

    public static function getStatisticsDomain($type = null){
        if($type){
            if(isset(self::$stats_domains[$type])){
                return self::$stats_domains[$type].'/';
            }
        }
        return null;
    }
    
    public static function renderCombinationNotifications(array $offerData){
        $html = '';
        if(isset($offerData['combtosamearea']) && $offerData['combtosamearea']){
            $html = ' **';
        }elseif(isset($offerData['combtosamemodule']) && $offerData['combtosamemodule']){
            $html = ' *';
        } 
        return $html;
    }
    
    public static function getMatchPagePatternTitle($odds_title){
        $cor_pattern = explode('|', $odds_title);  
        end($cor_pattern); 
        return key($cor_pattern);
    }
}
