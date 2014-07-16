<tr>
    <td class="date"><?=OddsViewHelper::formatResultsDate($date)?><br><span class="name">#<?=$name?></span></td>
    <td class="betsPrefix"><?=$title?></td>
    <td class="result"><?=$result?></td>
    <td class="won-selections"><?php if(count($selections)):?><?=implode(',<br>', $selections)?>.<?php endif; ?></td>
    <td class="more"><?php if($advanced>0):?><a href="<?=$path?>/<?=$id?>">+<?=$advanced?></a><?php endif; ?></td>
</tr>