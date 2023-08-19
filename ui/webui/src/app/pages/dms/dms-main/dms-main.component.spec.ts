import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DmsMainComponent } from './dms-main.component';

describe('DmsMainComponent', () => {
  let component: DmsMainComponent;
  let fixture: ComponentFixture<DmsMainComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [DmsMainComponent]
    });
    fixture = TestBed.createComponent(DmsMainComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
