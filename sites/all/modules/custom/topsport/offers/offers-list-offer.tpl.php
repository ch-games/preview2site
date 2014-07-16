<?php 
$offerData['#category'] = htmlspecialchars($offerData['#category']);
?>
<tr id="e_<?php print $offerData['#id']; ?>"
	class="<?php print (count($offerData['#odds']) > 1 ? "offers_line" : "");?>"
	title="<?php print $offerData['#category'].t(', įvykio nr. @number', array('@number' => $offerData['#name'])); ?>">
	<td class="date"><?php print offers_dateTimeFormat($offerData['#date']); ?>
	</td>
	<?php if(isset($offerData['#hidename']) && !$offerData['#hidename']): ?>
	<td class="number"><?php print $offerData['#name']; ?>
	</td>
	<?php endif; ?>
	<?php
	$html = '';
	$html .= '<td class="odd_title"> '.$offerData['#title'].' </td>';
	foreach($offerData['#odds'] as $odds){
		$html .= '<td id="'.'e_'.$offerData['#id'].'_'.$odds['#code'].'" '.($odds['#rate'] > 1 ? 'onclick="_gaq.push([\'_trackEvent\', \'OddsClassic\', \''.$offerData['#category'].'\', \'BetsSlip: '.$odds['#code'].'\']); Drupal.betAction(this, '.$offerData['#id'].', '.$odds['#code'].');"' : '').' class="'.($odds['#rate'] > 1 ? 'betaction' : '').'">'.($odds['#rate'] > 1 ? '<span class="rate'.($odds['#rate_status'] && $eid ? ($odds['#rate_status'] == 1 ? ' up' : ' down') : '').'">'.$odds['#rate'].'</span>' : '').'</td>';
	}
	print $html;
	?>    
	<td class="more<?=($eid ? ' empty' : '')?>" align="center"><?php print ($offerData['#nid'] ? l('+'.$offerData['#child_cnt'], 'node/'.$offerData['#nid'], array('query' => array('full' => 1))) . offers_stats_icon($offerData['#id']) /*. offers_enetpulse_icon($offerData['#id'], 'node/'.$offerData['#nid'])*/ : ''); ?></td>
</tr>
