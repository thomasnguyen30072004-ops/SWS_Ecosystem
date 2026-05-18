#include "servo_control.h"
#include "app_config.h"
#include "driver/mcpwm_prelude.h"

static mcpwm_cmpr_handle_t comparators[2];

void servo_init_hardware(void) {
    // 1. Cấu hình Timer: Tần số 50Hz (Chu kỳ 20ms)
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t t_cfg = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000, // 1us mỗi tick
        .period_ticks = 20000,    // 20ms
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    mcpwm_new_timer(&t_cfg, &timer);

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_new_operator(&(mcpwm_operator_config_t){.group_id = 0}, &oper);
    mcpwm_operator_connect_timer(oper, timer);

    // 2. Khởi tạo bộ phát xung cho 2 chân GPIO (PAN và TILT)
    int gpios[] = {SERVO_PAN_GPIO, SERVO_TILT_GPIO};
    for(int i=0; i<2; i++) {
        mcpwm_gen_handle_t gen = NULL;
        mcpwm_new_generator(oper, &(mcpwm_generator_config_t){.gen_gpio_num = gpios[i]}, &gen);
        mcpwm_new_comparator(oper, &(mcpwm_comparator_config_t){.flags.update_cmp_on_tez = true}, &comparators[i]);
        
        mcpwm_generator_set_action_on_timer_event(gen, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
        mcpwm_generator_set_action_on_compare_event(gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparators[i], MCPWM_GEN_ACTION_LOW));
    }
    
    mcpwm_timer_enable(timer);
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}

void servo_write_angle(int id, int angle) {
    // Giới hạn an toàn từ 0 đến 180 độ
    if (angle < 0) angle = 0; 
    if (angle > 180) angle = 180;

    /* 
       Công thức chuyển đổi:
       0 độ   -> 500us
       180 độ -> 2500us
       Dải xung (Range) = 2000us
    */
    uint32_t duty = (angle * 2000 / 180) + 500;
    mcpwm_comparator_set_compare_value(comparators[id], duty);
}