(function($) {

	try {
		Drupal.betslip = {};
		Drupal.betslip.init = true;

		function resizeBetSlipBlock() {
			var bsBlk = $('#block-betslip-betslip'), hw = $(window).height();

			// betslip_messages
			if (Drupal.betslip_hide_timer) {
				clearTimeout(Drupal.betslip_hide_timer);
				Drupal.betslip_hide_timer = false;
			}
			if ($('#betslip_empty', bsBlk).length)
			{
				var to = $('div.messages', $('#betslip_messages', bsBlk)).length ? 15000 : 5;
				Drupal.betslip_hide_timer = setTimeout(function() {
					Drupal.betslip_hide_timer = false;
					$('#betslip_types_list', bsBlk).hide();
					$('#betslip_bottom_info', bsBlk).hide();
					$('#betslip_actions', bsBlk).hide();
					$('.block-content ul:first', bsBlk).hide();
				}, to);
			} else {
				$('#betslip_bottom_info', bsBlk).show();
				$('#betslip_actions', bsBlk).show();
				$('.block-content ul:first', bsBlk).show();
			}

			if (bsBlk.hasClass('betslipfloat')) {
				if ($('#betslip_rows', bsBlk).length) {
					if (bsBlk.height() > hw)
						bsBlk.removeClass('betslipfloat');

				} else {
					bsBlk.removeClass('betslipfloat');
					$(window).scrollTop(0);
				}
			} else {
				var st = $(window).scrollTop();

				if (st > 300 && hw > bsBlk.height()) {
					bsBlk.addClass('betslipfloat');
				}
			}
		}

		function getMessageType(type) {
			if (type == 'error')
				type = 'danger';
			type = 'alert alert-dismissable alert-' + type;
			return type;
		}
		Drupal.ajax.prototype.commands.betlipmessage = function(ajax, response, status) {
			setTimeout(function() {
				$('#betslip_messages').append(
						'<div class="messages ' + getMessageType(response.type) + '"><button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>'
						+ response.message + '</div>');
				resizeBetSlipBlock();
			}, 100);
		};

		Drupal.ajax.prototype.commands.changedKoef = function(ajax, response, status)
		{

			$('#betslip_rows .slip_row').each(function() {
				var r = $(this).attr('id').split('_');
				//alert(r[1] + ' == ' + response.eid);
				if (r[1] == response.eid)
				{
					var c = 'down';
					if (parseFloat(response.nCoef) > parseFloat(response.oCoef))
						c = 'up';
					$('.rate', $(this)).html(response.nCoef).addClass(c);
				}

			});
			//alert(JSON.stringify(response));
		};

		Drupal.convertToFloat = function(strNumber) {
			strNumber = String(strNumber);
			var curNumber = 0.00;
			intDecimalPointPos = strNumber.indexOf(',');
			if (intDecimalPointPos > 0) {
				strNumber = strNumber.substring(0, intDecimalPointPos)
						+ '.'
						+ strNumber.substring(intDecimalPointPos + 1,
								strNumber.length);
			}
			if (strNumber.indexOf('.') > 0) {
				while (strNumber.substring(strNumber.length - 1,
						strNumber.length) == '0') {
					strNumber = strNumber.substring(0, strNumber.length - 1)
				}
			}
			if (strNumber.substring(strNumber.length - 1, strNumber.length) == '.') {
				strNumber = strNumber.substring(0, strNumber.length - 1)
			}
			curNumber = parseFloat(strNumber);
			if (curNumber != strNumber) {
				curNumber = 0.00;
			}
			return curNumber;
		}

		Drupal.cleanStakeFocus = function(element) {
			element = $(element);
			var stake = Drupal.convertToFloat(element.val()).toFixed(2);
			if (stake == 0.00) {
				element.val('');
			}
		}

		Drupal.cleanStakeBlur = function(element) {
			element = $(element);
			var stake = Drupal.convertToFloat(element.val()).toFixed(2);
			if (stake == 0.00) {
				element.val('0.00');
			}
		}

		Drupal.constantChange = function(element) {
			var contsCount = $(element).val();
			$('#betslip_rows .slip_row').css('background-image', '');
			$(
					'#betslip_rows .slip_row:not(.ui-sortable-helper):lt('
					+ contsCount + ')').each(
					function() {
						$(this).css('background-image', 'none');
						if ($(this).hasClass('ui-sortable-placeholder')) {
							$('#betslip_rows .slip_row.ui-sortable-helper')
									.css('background-image', 'none');
						}
					});
		}

		Drupal.regenerateLetters = function() {
			var i = 0;
			var letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
			$('#betslip_rows .slip_row .row-letter').each(function() {
				$(this).html(letters.substr(i++, 1));
			})
		}

		Drupal.combinationsToggle = function(element) {
			$(element).parent().parent().toggleClass("expand");
		}

		Drupal.betAction = function(element, event, choise, ga_context) {
			$(element).effect("transfer", {
				to: $("#betslip_rows")
			}, 500);
			var base = $(element).attr('data-eid') || $(element).attr('id');
			if (!$(element).hasClass('ajax-processed'))
			{
				$(element).addClass('ajax-processed');
				var element_settings = {};
				element_settings.url = Drupal.settings.betslip['action_toggle']
						+ '/' + event + '/' + choise + '/';
				element_settings.event = 'betActionEvent';
				element_settings.effect = 'fade';
				element_settings.progress = {};
				Drupal.ajax[base] = new Drupal.ajax(base, element,
						element_settings);
				$(element).unbind('betActionEvent');
				Drupal.ajax[base].eventResponse(Drupal.ajax[base]);
				$(element).removeClass('ajax-processed');
				// GA tracking
				if (ga_context && _gaq) {
					trackEvent = ['_trackEvent'];
					trackEvent.push('OddsEU');
					trackEvent.push(ga_context);
					trackEvent.push('BetsSlip: ' + choise);
					_gaq.push(trackEvent);
				}
			}

			//$(element).trigger('betActionEvent');
		}

		Drupal.behaviors.betslip = {
			attach: function(context, settings) {
				$(
						'#betslip_bottom form input[name="betslip_stake"]:not(.keyup-processed)',
						context).addClass('keyup-processed').keyup(
						function() {
							var rate_total = Drupal.convertToFloat(
									Drupal.settings.betslip['rate_total'])
									.toFixed(2);
							var stake = Drupal.convertToFloat($(this).val())
									.toFixed(2);
							var win_total = Drupal.convertToFloat(
									rate_total * stake).toFixed(2);
							$('#win_total').html(win_total);
							var bets_total = Drupal.convertToFloat(
									Drupal.settings.betslip['bets_total'])
									.toFixed(2);
							var stake_total = Drupal.convertToFloat(
									bets_total * stake).toFixed(2);
							$('#betslip_stake_total').html(stake_total);
						});
				$('#betslip_rows', context)
						.sortable(
								{
									handle: '.handle',
									item: '.slip_row',
									axis: 'y',
									cursor: 'move',
									update: function(event, ui) {
										Drupal.regenerateLetters();
										Drupal.ajax['betslip_rows_rearrange'].options.data['slip_ids[]'] = [];
										$('#betslip_rows .slip_row')
												.each(
														function() {
															Drupal.ajax['betslip_rows_rearrange'].options.data['slip_ids[]']
																	.push(this.id);
														});
										$('#betslip_rows').trigger('rearrange');
									},
									change: function(event, ui) {
										Drupal
												.constantChange($('#system_bets_add_form select[name="constant"]'));
									},
									stop: function(event, ui) {
										Drupal
												.constantChange($('#system_bets_add_form select[name="constant"]'));
									}
								});
				$('#betslip_rows:not(.ajax-processed)', context)
						.addClass('ajax-processed')
						.each(
								function() {
									var element_settings = {};
									var str = this.id;
									var ids = str.split('_');
									element_settings.url = Drupal.settings.betslip['action_rearrange'];
									element_settings.event = 'rearrange';
									element_settings.effect = 'fade';
									element_settings.progress = {};
									var base = $(this).attr('id');
									Drupal.ajax['betslip_rows_rearrange'] = new Drupal.ajax(
											base, this, element_settings);
								});
				Drupal
						.constantChange($('#system_bets_add_form select[name="constant"]'));
				Drupal.regenerateLetters();
				setTimeout(resizeBetSlipBlock, 100);
				if (Drupal.betslip.init) {
					Drupal.betslip.init = false;
					$.each(Drupal.settings.betslip['bets_active'], function(
							index, value) {
						$('#e_' + value).addClass('active');
						$('[data-eid="' + value + '"]').addClass('active');
						var substr = value.split('_');
						$('#ball_' + substr[0]).addClass('bs' + substr[1]);
					});
				}
			}
		};
		/* bet from link */
		$(document)
				.ready(
						function() {
							$(document)
									.delegate(
											'#sharebetslink',
											'click',
											function(e) {
												e.preventDefault();
												var url = document.location.href, ps = url
														.indexOf('#');
												if (ps > 0)
													url = url.substr(0, ps);
												url += $(this).attr('data-ts-link');

												var dl = $('#sharelinkdialog');
												if (!dl.length) {
													dl = $('<div id="sharelinkdialog"><input /><div></div></div>');
													$('input', dl).css({
														width: '99%',
														fontSize: '20px'
													});

												}
												dl
														.dialog({
															width: 500,
															height: 135,
															title: Drupal.settings.betslip['share_link_title']
														});
												var dt = new Date(
														(new Date().getTime() + parseInt($(
																this).attr(
																'data-ts-exptime')) * 1000)), hh = dt
														.getHours(), mm = dt
														.getMinutes(), yy = 1900 + dt
														.getYear(), mo = dt
														.getMonth() + 1, dd = dt
														.getDate(), date = yy
														+ '-'
														+ (mo < 10 ? '0' + mo
																: mo)
														+ '-'
														+ (dd < 10 ? '0' + dd
																: dd)
														+ ' '
														+ (hh < 10 ? '0' + hh
																: hh)
														+ ':'
														+ (mm < 10 ? '0' + mm
																: mm);

												var social = '<div class="social-share"><span class="fb-like"><div class="fb-share-button" data-href=" ' + url + ' " data-type="button"></div></span>' + '<span class="g-like"><g:plus action="share" data-href="' + url + '"></g:plus></span></div>';

												$('div', dl)
														.html(
																Drupal.settings.betslip['share_link_description']
																.split(
																		'%date')
																.join(
																		date) + social);
												$('input', dl).val(url)
														.select();


												(function(doc, script) {
													console.log(doc.createDocumentFragment());
													var js,
															fjs = doc.getElementsByTagName(script)[0],
															frag = doc.createDocumentFragment(),
															add = function(url, id) {
																if (doc.getElementById(id)) {
																	return;
																}
																js = doc.createElement(script);
																js.src = url;
																id && (js.id = id);
																frag.appendChild(js);
															};

													// Google+ button
													add('//apis.google.com/js/plusone.js');
													fjs.parentNode.insertBefore(frag, fjs);
												}(document, 'script'));
												$(document).ready(function() {
													$.ajaxSetup({cache: true});
													$.getScript('//connect.facebook.net/en_US/all.js', function() {
														FB.init({
															appId: '200103733347528',
															xfbml: true
														});
													});
												});
												jQuery('.fb-like').html('<div class="fb-share-button" data-href=" ' + url + ' " data-type="button">');
												jQuery('.g-like').html('<g:plus action="share" data-href="' + url + '"></g:plus>');


											});

							function addBets(d) {

								if (_gaq)
									_gaq.push(['_trackEvent',
										'dalintiskortele',
										'nuorodos naudojimas',
										'korteles nuoroda']);

								var element_settings = {};
								element_settings.url = Drupal.settings.betslip['action_addbets']
										+ '/' + d;
								element_settings.event = 'betAction';
								element_settings.effect = 'fade';
								element_settings.progress = {};

								Drupal.ajax['custom_add'] = new Drupal.ajax(
										null, $(document.body),
										element_settings);

								$(document.body).trigger('betAction');

							}

							var apos = document.location.href.indexOf('#', 0);
							if (apos) {
								var hm = document.location.href
										.substr(apos + 1);
								if (hm.substr(0, 4) == 'bet-') {
									var d = hm.substr(4);
									addBets(d);
								}
							}
							// adding tooltip to titles
							$('body').tooltip({selector: '[data-toggle="tooltip"]', trigger: 'click hover focus', delay: {show: 400, hide: 100}});
						});

		/* Auto resize */
		$(window)
				.scroll(
						function() {
							var bsBlk = $('#block-betslip-betslip'), hw = $(window).height();
							var st = $(window).scrollTop();
							if (st > 300) {
								if (!$('#betslip_empty').length) {
									if (hw < bsBlk.height()) {
										bsBlk.removeClass('betslipfloat');
									} else {
										bsBlk.addClass('betslipfloat');
									}
								} else {
									bsBlk.removeClass('betslipfloat');
								}
							} else {
								bsBlk.removeClass('betslipfloat');
							}
						}).resize(function() {
			resizeBetSlipBlock();
		});
		/* end of auto-resize */
	} catch (err) {
	}
})(jQuery);
