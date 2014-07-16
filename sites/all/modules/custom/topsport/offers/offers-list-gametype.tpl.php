<div class="offer<?= ($gameTypeCollapsed && $eid ? ' collapsed' : '')?>">
	<h5 id="<?=$title;?>" data-gtid="<?=$gameType_id?>" title="<?=t('Paspauskite norint suskleisti/išskleisti statymo tipą')?>"><?=strtr($title, array('{'=>'(', '}'=>')'))?></h5>
    <div class="offers_list-odds">
		<table
			class="odds<?php print (count($coefNames) == 1 ? ' offers_line' : ''); ?>">
			<tr>
				<th colspan="<?php print(!$loged?3:2)?>"></th>
				<?php
				foreach($coefNames as $name): ?>
				<th class="odd_title" title="<?=t('Baigtis '.$name)?>"><?php print $name; ?>
				</th>
				<?php endforeach; ?>
			</tr>
			<?php print $content; ?>
		</table>
		<div class="offers_list-rules">
		<?php if(isset($combtosamemodule) AND $combtosamemodule): ?>
		<?php print t('* - Įvykiai negali būti kombinuojami su ta pačia lyga').'<br/>'; ?>
		<?php endif; ?>
		<?php if(isset($combtosamearea) AND $combtosamearea): ?>
		<?php print t('** - Įvykiai negali būti kombinuojami su ta pačia sporto šaka ir lyga').'<br/>'; ?>
		<?php endif; ?>
		</div>
	</div>
</div>
