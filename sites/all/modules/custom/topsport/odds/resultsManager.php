<?php
namespace oddsManager;

require_once drupal_get_path('module', 'odds') . '/oddsManager.php';


/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
class ResultsManager extends OddsManager
{
	public function getResults( $lang, $args )
	{
		switch (count($args))
		{
			case 0: // Filters
				return $this->filters($lang);
			case 1:	// Areas
				return $this->getAreas($lang, $args[0]);
			case 2: // Countries
				return $this->getCountries($lang, $args[0], $args[1]);
			case 3:	// Modules
				return $this->getModules($lang, $args[0], $args[1], $args[2]);
			case 4:	// Events
				return $this->getEvents($lang, $args[0], $args[1], $args[2], $args[3]);
			case 5: // Match
				return $this->getMatch($lang, $args[0], $args[1], $args[2], $args[3], $args[4]);
		}
		return array();
	}
	
	private $date_from;
	private $date_to;
	
	public function setTimeInterval($date_from, $date_to)
	{
		$this->date_from = $date_from;
		$this->date_to = $date_to;
	}
	
	protected function filters($lang)
	{
		$titles = array(			
			'today'=>t('Šiandien'),
			'yesterday' => t('Vakar'),
			'week' => t('Paskutinės 7 dienos')
		);
		
				
		$r = array();
		foreach ($titles as $id => $title)
		{
			$r[] = array('type'=>'category','id'=>$id, 'title'=>$title);
		}
		return $r;
	}

	protected function getQueryFilter($type, $append = false)
	{
		$prefix = $append ? ' and' : '';
		
		switch ($type)
		{
			case 'today' :
				$this->date_from = strtotime('-24 hours');
				$this->date_to = strtotime('tomorrow');
				break;
			case 'yesterday' :
				$this->date_from = strtotime('-48 hours');
				$this->date_to = strtotime('today');
				break;
			case 'week' :
				$this->date_from =  strtotime('-7 days');
				$this->date_to = strtotime('tomorrow');
				break;
			
		}
		
		if(isset($this->date_from) && isset($this->date_to))
		{
			return $prefix . ' o_r.date between ' . $this->date_from . ' and ' . $this->date_to;
		}
		
	}

	protected function getAreas($lang, $type)
	{
		$db = $this->getConnection();
		
		$query = 'select distinct on (aid) aid as area_id, area as title from ts_offers_results o_r where language = $1 ' . $this->getQueryFilter($type, true) ;
		$query =  'Select * FROM ('.$query.') t Order by area_id';
        $result = pg_query_params($db, $query, array($lang));
		
		$rez = array();
		
		while( $r = pg_fetch_assoc($result) )
		{			
			$rez[] = array('type'=>'category','id'=> $r['area_id'], 'title'=>trim($r['title']));
		}
		return $rez;
	}
	
	protected function getCountries($lang, $type, $aid)
	{
		$db = $this->getConnection();
		
		$query = 'select distinct on (cid) cid as country_id, country as title from ts_offers_results o_r where language = $1 and aid = $2 ' . $this->getQueryFilter($type, true) ;
		
		$result = pg_query_params($db, $query, array($lang, $aid));
		$rez = array();
		while ($r = pg_fetch_assoc($result))
		{			
			$rez[] = array('type'=>'category','id'=> $r['country_id'], 'title'=>trim($r['title']));			
		}
		return $rez;
	}
	
	private function getModules($lang, $type, $aid, $cid)
	{
		$db = $this->getConnection();	
		
		$query = 'select distinct on (mid) mid as module_id, module as title from ts_offers_results o_r where language = $1 and aid = $2 and cid = $3 ' . $this->getQueryFilter($type, true) ;
		
		//$query = 'select mid, category from ts_offers_events where eid in ( select max(eid) from ts_offers_results where  aid = $1 and cid = $2 group by mid)';
		$rez = array();
		if($result = pg_query_params($db, $query, array($lang, $aid, $cid)))
		{
			if($rows = pg_fetch_all($result))					
			foreach ($rows as $r)
			{			
				$rez[] = array('type'=>'category','id'=> $r['module_id'], 'title'=>trim($r['title']));			
			}
		}		
		return $rez;
	}
	
	public function findByName($name, $is_event = false)
	{
        global $language;
        if($is_event){
            $nametype = 'eid';
        }else {
            $nametype = 'name';
        }
		$db = $this->getConnection();
		$query = 'select eid, aid, cid, mid, area, country, module from ts_offers_results where language = $2 AND '.$nametype.' = $1 ORDER BY date DESC';
		if($result = pg_query_params($db, $query, array($name, $language->language)))
			return pg_fetch_object($result);
		return false;
	}
    
    public function findBy($qry_where, $args = array())
	{
        $db = $this->getConnection();
		$query = 'select eid, aid, cid, mid, area, country, module from ts_offers_results where '.$qry_where;
		if($result = pg_query_params($db, $query, $args))
			return pg_fetch_object($result);
		return false;
	}

	private function getEvents($lang, $type, $aid, $cid, $mid, $eid = false)
	{        
		$db = $this->getConnection();
		
		if($eid)
		{
			$query = 'select o_r.eid, o_r.peid, o_r.gtid, o_r.title, o_r.name, o_r.date, o_r.item1name, o_r.item2name, o_r.r1, o_r.r2, o_r.selections, o_r.totalforaresult1, o_r.totalforaresult2, 0 as cnt'
				. ' from ts_offers_results o_r where o_r.language = $1 and o_r.aid = $2 and o_r.cid = $3 and o_r.mid = $4 and (o_r.peid = $5 OR o_r.eid = $5)' . $this->getQueryFilter($type, true); 
			//$query .= ' Order By date DESC ';	
			$result = pg_query_params($db, $query, $_args = array($lang, $aid, $cid, $mid, $eid));
		}
		else
		{
			$query = 'select o_r.eid, o_r.peid, o_r.gtid, o_r.title, o_r.name, o_r.date, o_r.item1name, o_r.item2name, o_r.r1, o_r.r2, o_r.selections, o_r.totalforaresult1, o_r.totalforaresult2, (SELECT count(*) FROM  ts_offers_results where peid = o_r.eid)/*o_r.count*/ as cnt'
					. ' from ts_offers_results o_r where o_r.language = $1 and o_r.aid = $2 and o_r.cid = $3 and o_r.mid = $4 and o_r.peid is null ' . $this->getQueryFilter($type, true) 
					. ' group by o_r.eid, o_r.peid, o_r.gtid, o_r.title, o_r.name, o_r.date, o_r.item1name, o_r.item2name, o_r.r1, o_r.r2, o_r.selections, o_r.totalforaresult1, o_r.totalforaresult2,o_r.count';
			$result = pg_query_params($db, $query, $argss = array($lang, $aid, $cid, $mid));
		}
		
		$events = array();
		$gt = array();
		$results = pg_fetch_all($result);        
		if($results)
		foreach ($results as $r)
		{			
			$events[] = $r;
			$gt[$r['gtid']] = $r['gtid'];
		}
		
		$gametypes = $this -> getGameTypes($gt, $lang);
		$gtpids = array();
		foreach ($gametypes as $gt)
			  $gtpids[$gt['pattern']] = $gt['pattern'];
		$patterns = $this -> getPatterns($gtpids, $lang);
		
		$rez = array();
		
		foreach ($events as $e)
		{
			$patdata = $this -> eventPatternData($e, $lang);
			
			$gt = $gametypes[$e['gtid']];
			$link = array('id'=>$e['eid'],
					'type' => 'result',
                    'date' => $e['date'],
                    'name' => $e['name'],
					'title' => $e['title'],
					'gametype' => $gt['title'],
					'advanced' => $e['cnt'],
					'selections' => array(),
					'result' => ($e['r1'] === 'Canceled' ? '<span class="label label-danger">'.t('Koeficientas prilygintas vienetui').'</span>' : '<span class="scores">'.$e['r1'].($e['r2'] != null ? ':'.$e['r2'] : '').'</span>')
				);
			
			$pattern = $patterns[$gt['pattern']];
			
			foreach (explode(',', $e['selections']) as $bet)
			{
				$bet = trim($bet);
				if(strlen($bet))
				{					
					for($i=0;$i<6;$i++)
						if(!strcmp ( $gt[$i] , $bet ))
						{                            
                            $_pattern = explode('|', $pattern[$i]);
                            $_pattern = end($_pattern);                           
                           
                            $_patdata = explode('|', $patdata['<E1>']);
                            $patdata['<E1>'] = end($_patdata);
							$link['selections'][] = strtr($_pattern, $patdata);
							break;
						}					
				}
			}			
			$rez[] = $link;
		}
				
		return $rez;
	}
	
	protected function getMatch($lang, $type, $aid, $cid, $mid, $eid)
	{
		return $this->getEvents($lang, $type, $aid, $cid, $mid, $eid);
	}
}
