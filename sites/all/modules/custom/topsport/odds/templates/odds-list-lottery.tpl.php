<div class="offer">
	<h5>
	<?php print $title; ?>
	</h5>
	<div class="offers_list-odds">
	<?php foreach($offerData as $ball): ?>
	<?php
	/*$ball_class = '';
	 foreach($ball['#odds'] as $key => $odd){
	 if($odd['#active']){
	 $ball_class .= ' '.'bs'.$odd['#code'];
	 }
	 }*/ ?>
		<div id="ball_<?php print $ball['#id'];?>" class="ball">
		<?php print $ball['#number']; ?>
		</div>
		<div class="ball-tooltip">
			<table class="odds">
			<?php foreach($ball['#odds'] as $key => $odd): ?>
				<tr>
					<td id="e_<?php print $ball['#id'].'_'.$odd['#code']; ?>"
						onclick="Drupal.betAction(this, <?php print $ball['#id'];?>, <?php print $odd['#code'];?>);"
						class="betaction"><?php print $odd['#rate'].' - '.$ball['#coefNames'][$odd['#title']];?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
		</div>

		<?php endforeach; ?>
	</div>
</div>




