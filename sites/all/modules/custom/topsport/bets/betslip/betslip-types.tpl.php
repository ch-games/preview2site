<?php if(!empty($items)) : ?>
<ul id="betslip_types_list" class="nav nav-tabs nav-justified">
<?php $i = 0; $cnt = count($items); ?>
<?php foreach($items as $key => $item) : $i++;?>
	<li
		class="item<?php print ($i == 1 ? ' first': '') ?><?php print ($i == $cnt ? ' last': '') ?><?php print ($item['active'] ? ' active': '') ?><?=(!$item['enabled'] ? ' disabled': '') ?>">
		<?php if(!$static):?><a class="use-ajax"
		href="<?php print url($item['link']); ?>"><?php print $item['title']?>
	</a> <?php else:?> <?php print $item['title']?> <?php endif; ?>
	</li>
	<?php endforeach; ?>
</ul>
<?php endif; ?>