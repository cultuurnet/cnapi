<?php

/**
 * @file
 * Handles some SEO optimisations for lists (contexts) and detail pages.
 */

/**
 * Preprocess variables for theme_link.
 *
 * @see theme_link()
 */
function cnapi_ui_preprocess_link(&$variables) {
  // Pase the link url as a Drupal cnapi request object.
  $request = cnapi_url_dua2dp(array('path' => $variables['path'], 'options' => $variables['options']));

  // Only act on cnapi list links.
  if ($request && isset($request['context'])) {
    $context = cnapi_ui_contexts('id', $request['context']);

    // In case the url has more parameters than page and heading, we add rel=nofollow. We only want search engines to index lists by heading.
    if (count(array_diff(array_keys($request['query']), array('page', 'heading', 'regio', 'cityid'))) > 0) {
      $variables['options']['attributes']['rel'] = 'nofollow';
    }
    elseif ($context['type'] == 'event' && !isset($request['query']['heading'])) {
      $variables['options']['attributes']['rel'] = 'nofollow';
    }

    if (isset($request['query']['heading'])) {
      $headings = $request['query']['heading'];
      $headings = explode(';', $headings);
      if (count($headings) > 1) {
        $variables['options']['attributes']['rel'] = 'nofollow';
      }
    }

    // We remove the "active" class on all cnapi list links.
    // Because all cnapi list links have the same path (but different querystring), Drupal marks all list links as active on all pages.
    $classes = isset($variables['options']['attributes']['class']) ? $variables['options']['attributes']['class'] : array();

    $classes = array_diff($classes, array('active'));

    unset($variables['options']['attributes']['class']);
    if (!empty($classes)) {
      $variables['options']['attributes']['class'] = $classes;
    }
  }
}

/**
 * Put noindex and canonical metatags for SEO on relevant cnapi pages.
 * Since we only want search engines to index listings by heading, we put a "noindex" on all other listings (ex. ?heading=123&cityid=9000_GENT).
 */
function cnapi_ui_set_canonical_and_noindex() {
  // Get active request.
  $request = cnapi_ui_get_active_request(FALSE);

  // We only act upon cnapi requests.
  if (!$request) {
    return;
  }

  // By default we allow robots to index and set no canonical.
  $noindex = FALSE;
  $canonical = FALSE;

  // If a context was set, we're on a list.
  if (isset($request['context'])) {
    // Fetch the context info
    $context = cnapi_ui_contexts('id', $request['context']);

    // And the query
    $query = $request['query'];

    // We only want to index the first page.
    if (isset($query['page']) && $query['page'] > 0) {
      $noindex = TRUE;
    }

    // If we're on an event list, we look at headings
    if ($context['type'] == 'event') {
      if (count(array_diff(array_keys($request['query']), array('page', 'heading', 'regio', 'cityid', 'k'))) > 0) {
        $noindex = TRUE;
      }

      if (isset($request['query']['heading'])) {
        $headings = $request['query']['heading'];
        $headings = explode(';', $headings);
        if (count($headings) > 1) {
          $noindex = TRUE;
        }

        // Don't allow combination of heading & keywords
        elseif (isset($request['query']['k'])) {
          $noindex = TRUE;
        }
        
        // Allow combination of regio and heading to be indexed  
        elseif (isset($request['query']['regio'])) {
          $regios = $request['query']['regio'];
          $regios = explode(';', $regios);
          if (count($regios) > 1) {
            $noindex = TRUE;
          }
          
          // We only want cities to be indexed, no provinces or touristic regions
          elseif ($regios[0] < 34 || $regios[0] > 348) {
            $noindex = TRUE;
          }
        }
      }

      // Allow keyword search to be indexed
      elseif (isset($request['query']['k'])) {
        $keywords = $request['query']['k'];
        $keywords = explode(';', $keywords);
        if (count($keywords) > 1) {
          $noindex = TRUE;
        }
      }
      else {
        $noindex = TRUE;
      }
    }

    // On actor and production listings we only allow the "page" parameter in the querystring.
    else {
      if (count(array_diff(array_keys($request['query']), array('page'))) > 0) {
        $noindex = TRUE;
      }
    }
  }

  // We don't allow indexing of passed events (older than 30 days).
  if (isset($request['event'])) {
    $event = cnapi_get_event($request['event']);
    if (isset($event['calendar']['stop'])) {
      if (REQUEST_TIME > $event['calendar']['stop'] + 60 * 60 * 24 * 30) {
        $noindex = TRUE;
      }
    }
  }

  // We only allow indexing of actor pages that are "assets".
  if (isset($request['actor'])) {
    $actor = cnapi_get_actor($request['actor']);
    if (!$actor['asset']) {
      $noindex = TRUE;
    }
  }

  // Add the "noindex, follow" meta tag if needed.
  if ($noindex) {
    $element = array(
      '#tag' => 'meta',
      '#attributes' => array(
        'name' => 'robots',
        'content' => 'noindex, follow',
      ),
    );

    drupal_add_html_head($element, 'nofollow');
  }

  // Add the rel="canonical" tag if needed.
  if ($canonical) {
    $canonical['options']['absolute'] = TRUE;
    drupal_add_html_head_link(array('rel' => 'canonical', 'href' => url($canonical['path'], $canonical['options'])), TRUE);
  }
}

/**
 * Detail page urls have a "slug". We don't care about the actual value of it since we only look at the cdbid in the 4th path argument. So agenda/e/abc/123 is the same as agenda/e/xyz/123.
 * To not be marked as duplicate content, we redirect urls like this to the one and only correct slug version, based on the object's title.
 */
function cnapi_ui_seo_redirects() {
  // Get active request.
  $request = cnapi_ui_get_active_request();

  // We only act upon cnapi requests.
  if (!$request) {
    return;
  }

  // We only act upon detail cnapi page request.
  $types = array_intersect(array_keys($request), array('actor', 'event', 'production'));
  $type = reset($types);
  if ($type) {
    if ($request[$type] == 'undefined') {
      return;
    }

    // Fetch the object's info from the api.
    $object = cnapi_get(cnapi_url_dp2p($request));

    if (empty($object)) {
      return;
    }

    // Add the title to the current $request.
    if (isset($object['detail']['nl']['title'])) {
      $request['title'] = $object['detail']['nl']['title'];
    }
    else {
      $request['title'] = $object['title'];
    }

    // Convert the $request to a Drupal url array. This will result in the one and only correct representation of the url of the current request.
    $url = cnapi_url_dp2dua($request);

    module_load_include('inc', 'cnapi_ui', 'cnapi_ui.hacks');

    // Compare the $url path part of the correct representation with the path of the current request.
    if ($url['path'] != request_path_raw()) {
      $params = drupal_get_query_parameters();
      cnapi_ui_goto($request, array('query' => $params));
    }
  }
}