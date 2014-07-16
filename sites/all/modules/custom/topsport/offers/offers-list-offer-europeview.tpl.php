<?php
$oddTotal = count($offerData['#odds']);
$rowspan = ($oddTotal > 3) ? 2 : 1;$rowspan = 1;
$offerData['#category'] = htmlspecialchars($offerData['#category']);
?>
<tr id="e_<?php print $offerData['#id']; ?>"
	class="<?php print (count($offerData['#odds']) > 1 ? "offers_line" : "");?>"
	title="<?php print $offerData['#category'].t(', įvykio nr. @number', array('@number' => $offerData['#name'])); ?>">
	<td rowspan="<?php print $rowspan;?>" class="date"><?php print offers_dateTimeFormat($offerData['#date']); ?>
	</td>
	<?php if(isset($offerData['#hidename']) && !$offerData['#hidename']): ?>
	<td rowspan="<?php print $rowspan;?>" class="number showalt"><?php print $offerData['#name']; ?>
	<?php if(isset($offerData['#combtosamemodule']) AND $offerData['#combtosamemodule']): ?>
	<?php print ' *'; ?> <?php endif; ?> <?php if(isset($offerData['#combtosamearea']) AND $offerData['#combtosamearea']): ?>
	<?php print ' **'; ?> <?php endif; ?></td>
	<?php endif; ?>
	<?php
	$html = '';
	$k = 0;
	$colspan = ($oddTotal > 3) ? 2 : (6 / $oddTotal);
	foreach($offerData['#odds'] as $odds){
		$cor_pattern = explode('|', $pattern->{'cn'.$odds['#code']});
		if($eid && count($cor_pattern) > 1)
		{
			$cor_pattern = $cor_pattern[1];
		}
		else
			$cor_pattern = $cor_pattern[0];
		if($eid && $pattern->cn1 == '<E1>' ){
			$e1 = explode(':', $offerData['#pattern']['<E1>']);
			if(count($e1) == 2) $offerData['#pattern']['<E1>'] = trim($e1[1]);
		}
		$html .= '<td colspan="'.$colspan.'" id="'.'e_'.$offerData['#id'].'_'.$odds['#code'].'" '.($odds['#rate'] > 1 ? 'onclick="_gaq.push([\'_trackEvent\', \'OddsEU\', \''.htmlspecialchars($offerData['#category']).'\', \'BetsSlip: '.$odds['#code'].'\']); Drupal.betAction(this, '.$offerData['#id'].', '.$odds['#code'].');"' : '').' class="'.($odds['#rate'] > 1 ? 'betaction' : '').'" style="width: auto;">'
		.'<div class="right rate'.($odds['#rate_status'] && $eid ? ($odds['#rate_status'] == 1 ? ' up' : ' down') : '').'" style="width: 40px;text-align: right;">'.($odds['#rate'] > 1 ? $odds['#rate'] : '').'</div>'
		.'<div style="margin-right: 40px;text-align: left;font-weight: normal;">'.(strtr($cor_pattern, $offerData['#pattern'])).'</div>'
		.'</td>';
		$k++;
		if($k == 3) { break; }
	}
	print $html;
	?>
	
    <td rowspan="<?php print $rowspan; ?>" class="icon showalt"><img class="more-icon<?=($oddTotal > 3 ? '' : ' inactive')?>" src="/sites/all/themes/topsport2/files/images/images/arrowdown<?=($oddTotal > 3 ? '' : '-inactive')?>.png" alt="" /></td>
	<td class="stats"><?= odds_stats_icon( $offerData['id'], $offerData['title']); ?></td>
    <td rowspan="<?php print $rowspan;?>" class="more<?=($eid ? ' empty' : '')?>" align="center"><?php print ($offerData['#nid'] ? l('+'.$offerData['#child_cnt'], 'node/'.$offerData['#nid'], array('query' => array('full' => 1))) /*. offers_stats_icon($offerData['#nid'], $offerData['#title'])*/ /*. offers_enetpulse_icon($offerData['#id'], 'node/'.$offerData['#nid']) */: ''); ?>
	</td>
</tr>
	<?php if($oddTotal > 3): ?>
<tr class="altrow" style="display: none;">
	<td colspan="2"></td>
	<?php
	$html = '';
	$l = 0;
	$oddTotalLeft = $oddTotal - $k;
	$colspan = (6 / $oddTotalLeft);
	foreach($offerData['#odds'] as $odds){
		$l++;
		if($l <= $k){ continue; }
		
		$cor_pattern = explode('|', $pattern->{'cn'.$odds['#code']});
		if($eid && count($cor_pattern) > 1)
		{
			$cor_pattern = $cor_pattern[1];
		}
		else
			$cor_pattern = $cor_pattern[0];
		
		$html .= '<td colspan="'.$colspan.'" id="'.'e_'.$offerData['#id'].'_'.$odds['#code'].'" '.($odds['#rate'] > 1 ? 'onclick="_gaq.push([\'_trackEvent\', \'OddsPage\', \''.$offerData['#category'].'\', \'BetsSlip: '.$odds['#code'].'\']); Drupal.betAction(this, '.$offerData['#id'].', '.$odds['#code'].');"' : '').' class="'.($odds['#rate'] > 1 ? 'betaction' : '').'" style="width: auto;">'
		.'<div class="right rate" style="width: 40px;text-align: right;">'.($odds['#rate'] > 1 ? $odds['#rate'] : '').'</div>'
		.'<div style="margin-right: 40px;text-align: left;font-weight: normal;">'.(strtr($cor_pattern, $offerData['#pattern'])).'</div>'
		.'</td>';
		$k++;
	}
	print $html;
	?>
	<td colspan="2" style="background: none; border-color: white;"></td>
</tr>
	<?php endif; ?>