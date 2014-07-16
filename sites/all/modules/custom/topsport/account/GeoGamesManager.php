<?php

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * Description of GeoGamesManager
 *
 * @author Irmantas
 */
class GeoGamesManager {

    /**
     * Every array key is country code every key element is equal for reg expression
     * @var array 
     */
    private static $restricted_geo_countries = array(
      'lt' => array(
        'casino-games',
        'live-casino',
        'financials\/trade',
        'racingdogs'
      )
    );
    
    private static $demoIps = array('88.119.21.64');
    private static $restricted_tsuids = array(45617,45834,49948,13602,23895);

    /**
     * 
     */
    public static function validatePage($user) {
        if (self::isGeoContryRestricted($user)) {
            drupal_set_message(t('Atsiprašome, deje negalime Jūsų aptarnauti...'), 'info');
            drupal_goto();
            // show content denied
        }
        // Permission show content passed
    }

    public static function showGeoContent($user) {
        if (self::isGeoContryRestrictedContent($user)) {
            // show content denied
            return false;
        }
        // Permission show content passed
        return true;
    }
    
    public static function showGeoContentCurrentUser(){
        global $user;
        return self::showGeoContent($user);
    }

    /**
     * 
     * @return boolean
     */
    private static function isGeoContryRestricted($user) {
        if (self::isDemoUser($user)) {
            return false;
        }
        $isrestricted = false;        
        $geo_country = geoip_country_code();
        $geo_country = strlen($geo_country) == 2 ? $geo_country : 'lt';
        $geo_country = strtolower($geo_country);
        if (isset(self::$restricted_geo_countries[$geo_country])) {
            $urls_patterns = implode('|', self::$restricted_geo_countries[$geo_country]);
            if (preg_match('/' . $urls_patterns . '/', current_path())) {
                return true;
            }
        }
        
        // its for restricted users
        if(self::isDisabledUser($user)){
            $geo_country = 'lt';
            if (isset(self::$restricted_geo_countries[$geo_country])) {
                $urls_patterns = implode('|', self::$restricted_geo_countries[$geo_country]);
                if (preg_match('/' . $urls_patterns . '/', current_path())) {
                    return true;
                }
            }
        }
        
        /*
         * DISABLED NATIONALITY VALIDATION
         * $user_country = self::getUserNationality($user);
        if ($user_country) {
            if (isset(self::$restricted_geo_countries[$user_country])) {
                $urls_patterns = implode('|', self::$restricted_geo_countries[$user_country]);
                if (preg_match('/' . $urls_patterns . '/', current_path())) {
                    return true;
                }
            }
        }*/
        return $isrestricted;
    }

        /**
     * 
     * @return boolean
     */
    private static function isGeoContryRestrictedContent($user) {
        $is_allowed = false;
        if (self::isDemoUser($user)) {
            return false;
        }
        
        $geo_country = geoip_country_code($_SERVER['REMOTE_ADDR']);
        $geo_country = strlen($geo_country) == 2 ? $geo_country : 'lt';
        $geo_country = strtolower($geo_country);
        if (isset(self::$restricted_geo_countries[$geo_country])) {
            return true;
        }
        
        if(self::isDisabledUser($user)){
            return true;
        }
        
       /* 
        * DISABLED NATIONALITY VALIDATION
        * $user_country = self::getUserNationality($user);        
        if ($user_country) {           
            if(isset(self::$restricted_geo_countries[$user_country])){
                return true;
            }
        }
        */

        return $is_allowed;
    }
    
    private static function getUserNationality($user) {
        $country = false;
        if (isset($user->data['personal_country']) && strlen($user->data['personal_country']) == 2) {
            $country = strtolower($user->data['personal_country']);
        }
        return $country;
    }

    /**
     * 
     * @param string $country_code
     * @param array $urls
     */
    public function setRestrictCountry($country_code, array $urls) {
        $country_code = strtolower($country_code);
        if (isset(self::$restricted_geo_countries[$country_code])) {
            self::$restricted_geo_countries[$country_code] += $urls;
        } else {
            self::$restricted_geo_countries[$country_code] = $urls;
        }
    }

    private static function isDemoUser($user) {
        /*if(in_array($_SERVER['REMOTE_ADDR'], self::$demoIps)){
            return true;
        }*/
        if (isset($user->data['tsuid']) && $user->data['tsuid'] == 726) {
            return true;
        }
        return false;
    }
    
    private static function isDisabledUser($user){
        if(isset($user->data['tsuid']) && in_array($user->data['tsuid'], self::$restricted_tsuids)){
            return true;
        }
        return false;
    }

}
