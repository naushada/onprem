import { ComponentFixture, TestBed } from '@angular/core/testing';

import { MainConfigComponent } from './main-config.component';

describe('MainConfigComponent', () => {
  let component: MainConfigComponent;
  let fixture: ComponentFixture<MainConfigComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [MainConfigComponent]
    });
    fixture = TestBed.createComponent(MainConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
