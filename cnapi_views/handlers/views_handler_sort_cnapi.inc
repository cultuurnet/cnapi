<?php

class views_handler_sort_cnapi extends views_handler_sort {

  /**
   * Determine if a sort can be exposed.
   */
  function can_expose() {
    return FALSE;
  }

  function query() {
    if ($this->options['sort'] == 'weightweek') {
      $this->query->add_orderby($this->options['sort']);
    }
    else {
      $this->query->add_orderby($this->options['sort'], $this->options['direction']);
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['sort'] = array('default' => '');
    $options['direction'] = array('default' => 'desc');

    return $options;
  }

  /**
   * Display whether or not the sort order is ascending or descending
   */
  function admin_summary() {
    if ($this->options['sort'] == 'weightweek') {
      return $this->options['sort'];
    }
    return $this->options['sort'] . ' ' . $this->options['direction'];
  }

  /**
   * Shortcut to display the value form.
   */
  function show_sort_form(&$form, &$form_state) {
    $form['sort'] = array(
      '#title' => t('How to sort'),
      '#type' => 'radios',
      '#options' => cnapi_get_sort_options(),
      '#default_value' => $this->options['sort'],
    );

    $form['direction'] = array(
      '#title' => t('Direction'),
      '#type' => 'radios',
      '#options' => cnapi_get_sort_direction_options(),
      '#default_value' => $this->options['direction'],
    );
  }

}